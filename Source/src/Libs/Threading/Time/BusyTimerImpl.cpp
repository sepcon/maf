#include "headers/Libs/Threading/Prv/Time/BusyTimerImpl.h"
#include <iostream>
#include <algorithm>
#include <sstream>

using namespace std::chrono;

#define LOG(message)

namespace thaf {
namespace threading {

struct JobDesc
{
    JobDesc(BusyTimerImpl::JobID id_, BusyTimerImpl::Duration duration_, BusyTimerImpl::TimeOutCallback callback_, bool isCyclic_ = false)
        : id(id_), startTime(system_clock::now()), duration(duration_), callback(callback_), isCyclic(isCyclic_)
    {}
    BusyTimerImpl::Duration remainTime()
    {
        return duration - elapseTime();
    }
    BusyTimerImpl::Duration elapseTime()
    {
        return duration_cast<milliseconds>(system_clock::now()- startTime).count();
    }
    void reset()
    {
        startTime = system_clock::now();
    }
    bool expired()
    {
        return elapseTime() >= duration;
    }

    BusyTimerImpl::JobID id;
    system_clock::time_point startTime;
    BusyTimerImpl::Duration duration;
    BusyTimerImpl::TimeOutCallback callback;
    bool isCyclic;
};

struct JobComp
{
    bool operator()(const BusyTimerImpl::JobDescRef& lhs,
        const BusyTimerImpl::JobDescRef& rhs) {
        return lhs->remainTime() > rhs->remainTime();
    }
};

BusyTimerImpl::~BusyTimerImpl()
{
    if(_future.valid())
    {
        _future.wait();
    } 
}

bool BusyTimerImpl::start(BusyTimerImpl::JobID jid, Duration ms, TimeOutCallback callback, bool cyclic)
{
    bool success = true;

    if (!_workerThreadIsRunning.load(std::memory_order_acquire))
    {
        std::lock_guard<std::mutex> lock(_jmt);
        startJob(jid, ms, callback, cyclic);
    }
    else
    {
        std::lock_guard<std::mutex> lock(_jmt);
        if(_pendingJobs.size() + _runningJobs.size() < MAX_JOBS_COUNT)
        {
            storePendingJob(jid, ms, callback, cyclic);
            _condvar.notify_one();
        }
        else
        {
            LOG("Cannot start timer for job " << jid << " Due to number of jobs exceeds the permited amount(" << MAX_JOBS_COUNT << "), please try with other BusyTimer");
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

void BusyTimerImpl::restart(BusyTimerImpl::JobID jid)
{
    std::unique_lock<std::mutex> lock(_jmt);
    auto job = getJob(_runningJobs, jid);
    if(job)
    {
        LOG("Timer " << jid << " is restarted with duration = " << (*itJob)->duration);
        job->reset();
        _condvar.notify_one();
    }
}

void BusyTimerImpl::stop(BusyTimerImpl::JobID jid)
{
    std::lock_guard<std::mutex> lock(_jmt);
    if (auto removedJob = removeJob(_runningJobs, jid))
    {
        LOG("Job " << jid << " is canceled");
        reorderRunningJobs();
        _condvar.notify_one();
    }
    else if(removeJob(_pendingJobs, jid))
    {
        LOG("Job " << jid << " is canceled");
    }
    else
    {
        LOG("Job " << jid << " does not exist or is already canceled");
    }
}

bool BusyTimerImpl::isRunning(BusyTimerImpl::JobID jid)
{
    std::lock_guard<std::mutex> lock(_jmt);
    bool running = false;
    if(!_shutdowned)
    {
        for(auto& job : _runningJobs)
        {
            if(job->id == jid)
            {
                running = true;
                break;
            }
        }
    }
    return running;
}

void BusyTimerImpl::setRecyclic(BusyTimerImpl::JobID jid, bool cyclic)
{
    std::lock_guard<std::mutex> lock(_jmt);
    auto itJob = getJob(_runningJobs, jid);
    if(itJob)
    {
        itJob->isCyclic = cyclic;
    }
    else
    {
        itJob = getJob(_pendingJobs, jid);
        if(itJob)
        {
            itJob->isCyclic = cyclic;
        }
    }
}

void BusyTimerImpl::shutdown()
{
    _shutdowned.store(true, std::memory_order_relaxed);
    _condvar.notify_one();
}

void BusyTimerImpl::startJob(BusyTimerImpl::JobID jid, BusyTimerImpl::Duration ms, BusyTimerImpl::TimeOutCallback callback, bool cyclic) noexcept
{
    addOrReplace(_runningJobs, std::make_shared<JobDesc>(jid, ms, callback, cyclic));
    reorderRunningJobs();
}

void BusyTimerImpl::reorderRunningJobs() noexcept
{
    std::make_heap(_runningJobs.begin(), _runningJobs.end(), JobComp());
}


void BusyTimerImpl::startPendingJobs() noexcept
{
    if(!_pendingJobs.empty())
    {
        for (auto& job : _pendingJobs)
        {
            addOrReplace(_runningJobs, std::move(job));
        }
        _pendingJobs.clear();
    }
}

void BusyTimerImpl::storePendingJob(JobID jid, Duration ms, TimeOutCallback callback, bool cyclic) noexcept
{
    storePendingJob(std::make_shared<JobDesc>(jid, ms, callback, cyclic));
}

void BusyTimerImpl::storePendingJob(BusyTimerImpl::JobDescRef job) noexcept
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

BusyTimerImpl::JobDescRef BusyTimerImpl::getJob(BusyTimerImpl::JobsContainer &jobs, BusyTimerImpl::JobID jid)
{
    auto itJob = findJob(jobs, jid);
    if(itJob != jobs.end())
    {
        return *itJob;
    }
    else
    {
        return nullptr;
    }
}

void BusyTimerImpl::run() noexcept
{
    _future = std::async(std::launch::async, [this] {
        _workerThreadIsRunning.store(true, std::memory_order_release);
        do
        {
            std::unique_lock<std::mutex> lock(_jmt);
            auto job = getShorttestDurationJob();

            if (!job) { break; }

            if(job->expired())
            {
                this->doJob(job);
                this->rescheduleShorttestJob();
            }
            else
            {
                if(_shutdowned.load(std::memory_order_acquire)) { break; }

                _condvar.wait_for(lock, milliseconds(job->remainTime()));

                if(_shutdowned.load(std::memory_order_acquire)) { break; }
                
                if(job->expired())
                {
                    this->doJob(job);
                    this->rescheduleShorttestJob();
                }
            }
            this->startPendingJobs();
            this->reorderRunningJobs();
        } while(!_shutdowned.load(std::memory_order_acquire));
        this->cleanup();
    });
}


BusyTimerImpl::JobDescRef BusyTimerImpl::getShorttestDurationJob() noexcept
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

void BusyTimerImpl::doJob(BusyTimerImpl::JobDescRef job)
{
    job->callback(job->id, job->isCyclic);
}

BusyTimerImpl::JobDescRef BusyTimerImpl::rescheduleShorttestJob() noexcept
{
    JobDescRef job;
    if(!_runningJobs.empty())
    {
        std::pop_heap(_runningJobs.begin(), _runningJobs.end(), JobComp());

        if(_runningJobs.back()->isCyclic)
        {
            _runningJobs.back()->reset();
            job = _runningJobs.back();
            std::push_heap(_runningJobs.begin(), _runningJobs.end());
        }
        else
        {
            job = std::move(_runningJobs.back());
            _runningJobs.pop_back();
        }
    }
    return job;
}

void BusyTimerImpl::cleanup()
{
    _shutdowned.store(true, std::memory_order_release);
    _workerThreadIsRunning.store(false, std::memory_order_release);
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
}
