#include "thaf/Threading/Prv/Time/TimerManagerImpl.h"
#include "thaf/Utils/Debugging/Debug.h"
#include <algorithm>

using namespace std::chrono;

#define LOG(message) thafInfo(message)

namespace thaf {
namespace threading {

struct JobDesc
{
    JobDesc(TimerManagerImpl::JobID id_, TimerManagerImpl::Duration duration_, TimerManagerImpl::TimeOutCallback callback_, bool isCyclic_ = false)
        : id(id_), startTime(system_clock::now()), duration(duration_), callback(callback_), isCyclic(isCyclic_)
    {}
    TimerManagerImpl::Duration remainTime()
    {
        return duration - elapseTime();
    }
    TimerManagerImpl::Duration elapseTime()
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

    TimerManagerImpl::JobID id;
    system_clock::time_point startTime;
    TimerManagerImpl::Duration duration;
    TimerManagerImpl::TimeOutCallback callback;
    bool isCyclic;
};

struct JobComp
{
    bool operator()(const TimerManagerImpl::JobDescRef& lhs,
        const TimerManagerImpl::JobDescRef& rhs) {
        return lhs->remainTime() > rhs->remainTime();
    }
};

TimerManagerImpl::~TimerManagerImpl()
{
    if(_future.valid())
    {
        _future.wait();
    } 
}

bool TimerManagerImpl::start(TimerManagerImpl::JobID jid, Duration ms, TimeOutCallback callback, bool cyclic)
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

void TimerManagerImpl::restart(TimerManagerImpl::JobID jid)
{
    std::unique_lock<std::mutex> lock(_jmt);
    auto job = getJob(_runningJobs, jid);
    if(job)
    {
        LOG("Timer " << jid << " is restarted with duration = " << job->duration);
        job->reset();
        _condvar.notify_one();
    }
}

void TimerManagerImpl::stop(TimerManagerImpl::JobID jid)
{
    std::lock_guard<std::mutex> lock(_jmt);
    if(removeJob(_pendingJobs, jid))
    {
        LOG("Job " << jid << " is canceled");
    }
    else if (auto removedJob = removeJob(_runningJobs, jid))
    {
        LOG("Job " << jid << " is canceled");
        reorderRunningJobs();
        _condvar.notify_one();
    }
    else
    {
        LOG("Job " << jid << " does not exist or is already canceled");
    }
}

bool TimerManagerImpl::isRunning(TimerManagerImpl::JobID jid)
{
    std::lock_guard<std::mutex> lock(_jmt);
    if(!_shutdowned)
    {
        auto jobFoundOn = [&jid](const JobsContainer& jobs) -> bool {
            for(auto& job : jobs)
            {
                if(job->id == jid)
                {
                    return true;
                }
            }
            return false;
        };
        return jobFoundOn(_pendingJobs) || jobFoundOn(_runningJobs);
    }
    return false;
}

void TimerManagerImpl::setRecyclic(TimerManagerImpl::JobID jid, bool cyclic)
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

void TimerManagerImpl::shutdown()
{
    _shutdowned.store(true, std::memory_order_relaxed);
    _condvar.notify_one();
}

void TimerManagerImpl::startJob(TimerManagerImpl::JobID jid, TimerManagerImpl::Duration ms, TimerManagerImpl::TimeOutCallback callback, bool cyclic) noexcept
{
    addOrReplace(_runningJobs, std::make_shared<JobDesc>(jid, ms, callback, cyclic));
    reorderRunningJobs();
}

void TimerManagerImpl::reorderRunningJobs() noexcept
{
    std::make_heap(_runningJobs.begin(), _runningJobs.end(), JobComp());
}


void TimerManagerImpl::startPendingJobs() noexcept
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

void TimerManagerImpl::storePendingJob(JobID jid, Duration ms, TimeOutCallback callback, bool cyclic) noexcept
{
    storePendingJob(std::make_shared<JobDesc>(jid, ms, callback, cyclic));
}

void TimerManagerImpl::storePendingJob(TimerManagerImpl::JobDescRef job) noexcept
{
    addOrReplace(_pendingJobs, std::move(job));
}

TimerManagerImpl::JobDescRef TimerManagerImpl::removeJob(TimerManagerImpl::JobsContainer& jobs, TimerManagerImpl::JobID jid)
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

TimerManagerImpl::JobsContainer::iterator TimerManagerImpl::findJob(TimerManagerImpl::JobsContainer &jobs, TimerManagerImpl::JobID jid)
{
    return std::find_if(jobs.begin(), jobs.end(),
                        [jid](const JobDescRef& job) { return job->id == jid; });
}

TimerManagerImpl::JobDescRef TimerManagerImpl::getJob(TimerManagerImpl::JobsContainer &jobs, TimerManagerImpl::JobID jid)
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

void TimerManagerImpl::run() noexcept
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


TimerManagerImpl::JobDescRef TimerManagerImpl::getShorttestDurationJob() noexcept
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

void TimerManagerImpl::doJob(TimerManagerImpl::JobDescRef job)
{
    job->callback(job->id, job->isCyclic);
}

TimerManagerImpl::JobDescRef TimerManagerImpl::rescheduleShorttestJob() noexcept
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

void TimerManagerImpl::cleanup()
{
    _shutdowned.store(true, std::memory_order_release);
    _workerThreadIsRunning.store(false, std::memory_order_release);
    _pendingJobs.clear();
    _runningJobs.clear(); //for shutting down case
}

void TimerManagerImpl::addOrReplace(TimerManagerImpl::JobsContainer &jobs, TimerManagerImpl::JobDescRef newJob)
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
