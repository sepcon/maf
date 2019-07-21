#include "thaf/threading/prv/TimerManagerImpl.h"
#include "thaf/utils/debugging/Debug.h"
#include <algorithm>
#include <cassert>

using namespace std::chrono;

#define LOG(message) thafInfo(message)

namespace thaf {
namespace threading {

static bool test(const std::atomic_bool& flag)
{
    return flag.load(std::memory_order_acquire);
}

static void set(std::atomic_bool& flag, bool value)
{
    return flag.store(value, std::memory_order_release);
}


template<typename Lockable>
class ConcurrentMutex
{
    Lockable& _l1;
    Lockable& _l2;
public:
    ConcurrentMutex(Lockable& l1, Lockable& l2) : _l1(l1), _l2(l2)
    {
    }
    void lock()
    {
        return std::lock(_l1, _l2);
    }
    void unlock()
    {
        _l1.unlock(); _l2.unlock();
    }
};

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
    void invalidate()
    {
        callback = nullptr;
    }
    bool valid() const
    {
        return bool(callback);
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

    if (!test(_workerThreadIsRunning))
    {
        startImmediately(jid, ms, callback, cyclic);
    }
    else
    {
        if(jobsCount() < MAX_JOBS_COUNT)
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

    if(success && test(_shutdowned))
    {
        set(_shutdowned, false);
        run();
    }
    return success;
}

void TimerManagerImpl::restart(TimerManagerImpl::JobID jid)
{
    std::unique_lock<JobsContainer> jobslock(_runningJobs);
    auto itJob = findJob__(_runningJobs, jid);
    if(itJob != _runningJobs->end())
    {
        LOG("Timer " << jid << " is restarted with duration = " << (*itJob)->duration);

        (*itJob)->reset();
        jobslock.unlock();
        _condvar.notify_one();
    }
}

void TimerManagerImpl::stop(TimerManagerImpl::JobID jid)
{
    if(removeAndInvalidateJob(_pendingJobs, jid))
    {
        LOG("Job " << jid << " is canceled");
    }
    else if(removeAndInvalidateJob(_runningJobs, jid))
    {
        LOG("Job " << jid << " is canceled");
        _condvar.notify_one();
    }
    else
    {
        assert(false);
        LOG("Job " << jid << " does not exist or is already canceled");
    }
}

bool TimerManagerImpl::isRunning(TimerManagerImpl::JobID jid)
{
    bool running = false;
    if(!test(_shutdowned))
    {
        auto jobFoundOn = [&jid](const JobsContainer& jobs) -> bool {
            auto lock(jobs.pa_lock());
            for(auto& job : *jobs)
            {
                if(job->id == jid)
                {
                    return true;
                }
            }
            return false;
        };

        running = (jobFoundOn(_pendingJobs) || jobFoundOn(_runningJobs));
    }

    return running;
}

void TimerManagerImpl::setCyclic(TimerManagerImpl::JobID jid, bool cyclic)
{
    auto setCyclic = [](JobsContainer& jobs, JobID jid, bool value) -> bool{
        auto lock(jobs.pa_lock());
        auto itJob = findJob__(jobs, jid);
        if(itJob != jobs->end())
        {
            (*itJob)->isCyclic = value;
        }
        return itJob != jobs->end();
    };

    if(!setCyclic(_pendingJobs, jid, cyclic))
    {
        setCyclic(_runningJobs, jid, cyclic);
    }
}

void TimerManagerImpl::shutdown()
{
    set(_shutdowned, true);
    _condvar.notify_one();
}

size_t TimerManagerImpl::jobsCount()
{
    std::scoped_lock lock(_pendingJobs, _runningJobs);
    return _pendingJobs->size() + _runningJobs->size();
}

void TimerManagerImpl::startImmediately(TimerManagerImpl::JobID jid, TimerManagerImpl::Duration ms, TimerManagerImpl::TimeOutCallback callback, bool cyclic) noexcept
{
    addOrReplace(_runningJobs, std::make_shared<JobDesc>(jid, ms, callback, cyclic));
}

void TimerManagerImpl::reorderRunningJobs__() noexcept
{
    std::make_heap(_runningJobs->begin(), _runningJobs->end(), JobComp());
}


void TimerManagerImpl::adoptPendingJobs__() noexcept
{
    if(!_pendingJobs->empty())
    {
        for (auto& job : *_pendingJobs)
        {
            addOrReplace(_runningJobs, std::move(job), false);
        }
        _pendingJobs->clear();
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

TimerManagerImpl::JobDescRef TimerManagerImpl::removeAndInvalidateJob(TimerManagerImpl::JobsContainer& jobs, TimerManagerImpl::JobID jid)
{
    auto lock(jobs.pa_lock());
    JobDescRef job;
    for(auto& j : *jobs)
    {
        if(j->id == jid)
        {
            job = std::move(j);
            job->invalidate();

            std::swap(j, jobs->back());
            jobs->pop_back();

            break;
        }
    }
    return job;
}

TimerManagerImpl::JobsIterator TimerManagerImpl::findJob__(TimerManagerImpl::JobsContainer &jobs, TimerManagerImpl::JobID jid)
{
    for(auto itJob = jobs->begin(); itJob != jobs->end(); ++itJob)
    {
        if((*itJob)->id == jid)
        {
            return itJob;
        }
    }
    return  jobs->end();
}

bool TimerManagerImpl::runJobIfExpired__(TimerManagerImpl::JobDescRef job)
{
    bool executed = false;
    if (job->expired())
    {
        LOG("Timer " << job->id << " expired!");
        try
        {
            if(job->valid()) // for case of job has been stopped when waiting to be expired
            {
                this->doJob__(job);
            }
        }
        catch(const std::exception& e)
        {
            LOG("Catch exception when executing job's callback: " << e.what());
        }
        catch(...)
        {
            LOG("Uncaught exception occurred when executing job's callback");
        }
        executed = true;
    }
    return executed;
}

std::unique_ptr<std::lock_guard<std::mutex>> TimerManagerImpl::autolock(TimerManagerImpl::JobsContainer &jobs, bool sync)
{
    if(sync)
    {
        return jobs.pa_lock();
    }
    else
    {
        return nullptr;
    }
}

void TimerManagerImpl::run() noexcept
{
    _future = std::async(std::launch::async, [this] {

        set(_workerThreadIsRunning, true);
        LOG("TimerManager thread is running!");
        do
        {
            ConcurrentMutex<JobsContainer> ccMutex(_pendingJobs, _runningJobs);
            std::unique_lock<decltype (ccMutex)> jobsLock(ccMutex);

            this->reorderRunningJobs__();
            auto job = getShorttestDurationJob__(_runningJobs);

			if (job)
			{
                if (runJobIfExpired__(job))
                {
                    this->scheduleShorttestJob__();
				}
				else
				{
					if (test(_shutdowned)) { break; }
					//TBD: _shutdowned might be set here
					_condvar.wait_for(jobsLock, milliseconds(job->remainTime()));

					if (test(_shutdowned)) { break; }

                    if (runJobIfExpired__(job))
                    {
                        this->scheduleShorttestJob__();
                    }
				}
			}
            else // there's no job to do
			{
//                LOG("Exit due to no more job to do");
//                break;
                _condvar.wait(ccMutex, [this]{
                    return (this->_pendingJobs->size() > 0) || (test(_shutdowned) == true);
                });
			}

            this->adoptPendingJobs__();

        } while(!test(_shutdowned));
        LOG("Timer manager thread is exitting");
        this->cleanup();
    });
}


TimerManagerImpl::JobDescRef TimerManagerImpl::getShorttestDurationJob__(const JobsContainer& jobs) noexcept
{
    if(jobs->empty())
    {
        return nullptr;
    }
    else
    {
        return jobs->front();
    }
}

void TimerManagerImpl::doJob__(TimerManagerImpl::JobDescRef job)
{
    job->callback(job->id, job->isCyclic);
}

TimerManagerImpl::JobDescRef TimerManagerImpl::scheduleShorttestJob__() noexcept
{
    JobDescRef job;
    if(!_runningJobs->empty())
    {
        std::pop_heap(_runningJobs->begin(), _runningJobs->end(), JobComp());

        if(_runningJobs->back()->isCyclic)
        {
            _runningJobs->back()->reset();
            job = _runningJobs->back();
            std::push_heap(_runningJobs->begin(), _runningJobs->end());
        }
        else // Remove job when it expired
        {
            job = std::move(_runningJobs->back());
            _runningJobs->pop_back();
        }
    }
    return job;
}

void TimerManagerImpl::cleanup()
{

    std::scoped_lock(_pendingJobs, _runningJobs);
    set(_workerThreadIsRunning, false);
    if(test(_shutdowned))
    {
        _pendingJobs->clear();
        _runningJobs->clear();
    }
    else
    {
        // due to the worker thread is no longer running, then _shutdowned flag must be set
        // to launch new worker thread later if client code invoke start function again
        set(_shutdowned, true);
    }
}

void TimerManagerImpl::addOrReplace(
        TimerManagerImpl::JobsContainer &jobs,
        TimerManagerImpl::JobDescRef newJob,
        bool sync)
{
    if(newJob)
    {
        auto lock = autolock(jobs, sync);
        auto itJob = findJob__(jobs, newJob->id);
        if(itJob != jobs->end())
        {
            *itJob = std::move(newJob);
        }
        else
        {
            jobs->emplace_back(std::move(newJob));
        }
    }
}

}
}
