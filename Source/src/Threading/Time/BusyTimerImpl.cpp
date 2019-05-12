#include "Prv/Time/BusyTimerImpl.h"
#include <iostream>
#include <algorithm>
#include <sstream>
using namespace std::chrono;

#define LOG(message)
//#define LOG(message) { \
//	std::stringstream singleStringMaker; \
//	singleStringMaker << message; \
//	std::cout << singleStringMaker.str() << std::endl; \
//}

namespace Threading
{

struct JobComp
{
    bool operator()(const BusyTimerImpl::JobDescRef& lhs,
        const BusyTimerImpl::JobDescRef& rhs) {
        return lhs->remainer > rhs->remainer;
    }
};

BusyTimerImpl::~BusyTimerImpl()
{
    if(_future.valid())
    {
        _future.wait();
    }
}

bool BusyTimerImpl::start(BusyTimerImpl::JobID tid, Duration ms, TimeOutCallback callback)
{
    bool success = true;

	if (!_workerThreadIsRunning.load(std::memory_order_acquire))
	{
		std::lock_guard<std::mutex> lock(_jmt);
		startJob(tid, ms, callback);
	}
    else
    {
        std::lock_guard<std::mutex> lock(_jmt);
        if(_pendingJobs.size() + _runningJobs.size() < MAX_JOBS_COUNT)
        {
            storePendingJob(tid, ms, callback);
            _condvar.notify_one();
        }
        else
        {
			LOG("Cannot start timer for job " << tid << " Due to number of jobs exceeds the permited amount(" << MAX_JOBS_COUNT << "), please try with other BusyTimer");
            success = false;
        }
    }

    if(_shutdowned.load(std::memory_order_acquire))
    {
        _shutdowned.store(false, std::memory_order_release);
        run();
    }
    return success;
}

void BusyTimerImpl::restart(BusyTimerImpl::JobID tid)
{
    std::unique_lock<std::mutex> lock(_jmt);
    auto itJob = findJob(_runningJobs, tid);
    if(itJob != _runningJobs.end())
    {
        LOG("Timer " << tid << " is restarted with duration = " << (*itJob)->duration);
        (*itJob)->reset();
        _condvar.notify_one();
    }
}

void BusyTimerImpl::stop(BusyTimerImpl::JobID tid)
{
    std::lock_guard<std::mutex> lock(_jmt);

    for(auto& job : _runningJobs)
    {
        LOG("id " << job->id);
    }
	if (removeJob(_runningJobs, tid))
	{
		LOG("Job " << tid << " is canceled");
		reorderRunningJobs();
		_condvar.notify_one();
	}
	else if(removeJob(_pendingJobs, tid))
	{
		LOG("Job " << tid << " is canceled");
	}
	else
	{
		LOG("Job " << tid << " does not exist or is already canceled");
	}
}

bool BusyTimerImpl::isRunning(BusyTimerImpl::JobID tid)
{
    std::lock_guard<std::mutex> lock(_jmt);
    bool running = false;
    if(!_shutdowned)
    {
        for(auto& job : _runningJobs)
        {
            if(job->id == tid)
            {
                running = true;
                break;
            }
        }
    }
    return running;
}

void BusyTimerImpl::shutdown()
{
    _shutdowned.store(true, std::memory_order_relaxed);
    _condvar.notify_one();
}

void BusyTimerImpl::startJob(BusyTimerImpl::JobID tid, BusyTimerImpl::Duration ms, BusyTimerImpl::TimeOutCallback callback)
{
    addOrReplace(_runningJobs, std::make_shared<JobDesc>(tid, ms, callback));
	reorderRunningJobs();
}

void BusyTimerImpl::reorderRunningJobs()
{
	std::push_heap(_runningJobs.begin(), _runningJobs.end());
}


void BusyTimerImpl::startPendingJobs()
{
    if(!_pendingJobs.empty())
    {
		for (auto& job : _pendingJobs)
		{
			addOrReplace(_runningJobs, std::move(job));
		}
        _pendingJobs.clear();
		reorderRunningJobs();
    }
}

void BusyTimerImpl::storePendingJob(JobID tid, Duration ms, TimeOutCallback callback)
{
    storePendingJob(std::make_shared<JobDesc>(tid, ms, callback));
}

void BusyTimerImpl::storePendingJob(BusyTimerImpl::JobDescRef job)
{
    addOrReplace(_pendingJobs, std::move(job));
}

BusyTimerImpl::JobDescRef BusyTimerImpl::removeJob(BusyTimerImpl::JobsContainer& jobs, BusyTimerImpl::JobID jid)
{
    auto itJob = findJob(jobs, jid);
	JobDescRef job;
    if (itJob != jobs.end())
    {
		job = *itJob;
		std::swap(*itJob, jobs.back());
		jobs.pop_back();
    }
	return job;
}

BusyTimerImpl::JobsContainer::iterator BusyTimerImpl::findJob(BusyTimerImpl::JobsContainer &jobs, BusyTimerImpl::JobID jid)
{
    return std::find_if(jobs.begin(), jobs.end(),
                        [jid](const JobDescRef& job) { return job->id == jid; });
}

void BusyTimerImpl::run()
{
    _future = std::async(std::launch::async, [this] {
		_workerThreadIsRunning.store(true, std::memory_order_release);
        do
        {
			Duration elapsedMS = 0;
            auto startTime = system_clock::now();
            std::unique_lock<std::mutex> lock(_jmt);
            auto job = getShorttestDurationJob();

			if (!job)
			{
				this->cleanup();
				break;
			}

            if(job->remainer <= 0)
            {
                this->doJob(job);
                this->popOutShottestJob();
            }
            else
            {
				if (_shutdowned.load(std::memory_order_acquire)) 
				{
					this->cleanup();
					break; 
				}

				job->remainer = job->remainer < 0 ? 0 : job->remainer;
                _condvar.wait_for(lock, milliseconds(job->remainer));
				
				if(_shutdowned.load(std::memory_order_acquire)) 
				{ 
					this->cleanup();
					break; 
				}
                
				else if(duration_cast<milliseconds>(system_clock::now() - startTime).count() >= job->remainer)
                {
                    this->doJob(job);
                    this->popOutShottestJob();
                }
                elapsedMS = duration_cast<milliseconds>(system_clock::now() - startTime).count();
                if(elapsedMS > 0)
                {
                    this->reEvaluateJobs(elapsedMS);
                    elapsedMS = 0;
                }
            }
            startPendingJobs();
        } while(true);
    });
}

bool BusyTimerImpl::jobRemained()
{
    std::lock_guard<std::mutex> lock(_jmt);
    return not _runningJobs.empty();
}

BusyTimerImpl::JobDescRef BusyTimerImpl::getShorttestDurationJob()
{
    if(_runningJobs.empty())
    {
        return nullptr;
    }
    else
    {
        return _runningJobs.front();
    }
}

void BusyTimerImpl::reEvaluateJobs(Duration elapsed)
{
    for(auto& job : _runningJobs)
    {
        job->remainer -= elapsed;
    }
}

void BusyTimerImpl::doJob(BusyTimerImpl::JobDescRef job)
{
    job->callback(job->id);
}

BusyTimerImpl::JobDescRef BusyTimerImpl::popOutShottestJob()
{
    std::pop_heap(_runningJobs.begin(), _runningJobs.end(), JobComp());
    auto job = std::move(_runningJobs.back());
    _runningJobs.pop_back();
    return job;
}

void BusyTimerImpl::cleanup()
{
	_shutdowned.store(true, std::memory_order_relaxed);
	_workerThreadIsRunning.store(false, std::memory_order_release);

	std::lock_guard<std::mutex> lock(_jmt);
	_pendingJobs.clear();
    _runningJobs.clear(); //for shutting down case
}

void BusyTimerImpl::addOrReplace(BusyTimerImpl::JobsContainer &jobs, BusyTimerImpl::JobDescRef newJob)
{
    if(newJob)
    {
        auto itJob = findJob(jobs, newJob->id);
        if (itJob != jobs.end())
        {
			LOG("Job " << newJob->id << " already existed in jobs store");
            (*itJob) = std::move(newJob);
        }
        else
        {
			LOG("New job added, id = " << newJob->id);
            jobs.push_back(std::move(newJob));
        }
    }
}

}
