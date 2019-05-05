#include "Prv/Time/BusyTimerImpl.h"
#include <iostream>

using namespace std::chrono;

namespace Threading
{

void BusyTimerImpl::start(BusyTimerImpl::TimerID tid, MS ms, TimeOutCallback callback)
{
    if(_shutdowned.load(std::memory_order_relaxed))
    {
        _shutdowned.store(false, std::memory_order_relaxed);
        {
            std::lock_guard<std::mutex> lock(_jmt);
            adoptJob(tid, ms, callback);
        }
        run();
    }
    else
    {
        storeJob(tid, ms, callback);
        _condvar.notify_one();
    }
}

void BusyTimerImpl::stop(BusyTimerImpl::TimerID tid)
{
    std::lock_guard<std::mutex> lock(_jmt);
    for(auto& job : _jobs)
    {
        std::cout << "id " << job.id << std::endl;
    }
    for(auto it = _jobs.begin(); it != _jobs.end(); ++it)
    {
        if(it->id == tid)
        {
            std::cout << "Job " << tid << " is canceled" << std::endl;
            _jobs.erase(it);
            _condvar.notify_one();
            break;
        }
    }
}

bool BusyTimerImpl::isRunning(BusyTimerImpl::TimerID tid)
{
    std::lock_guard<std::mutex> lock(_jmt);
    bool running = false;
    if(!_shutdowned)
    {
        for(auto& job : _jobs)
        {
            if(job.id == tid)
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

void BusyTimerImpl::adoptJob(BusyTimerImpl::TimerID tid, BusyTimerImpl::MS ms, BusyTimerImpl::TimeOutCallback callback)
{
    _jobs.insert(JobDesc(tid, ms, callback));
}

void BusyTimerImpl::adoptPendingJobs()
{
    if(!_pendingJob.empty())
    {
        for(auto& job : _pendingJob)
        {
            adoptJob(job.id, job.remainer, job.callback);
        }
        _pendingJob.clear();
    }
}

void BusyTimerImpl::storeJob(TimerID tid, MS ms, TimeOutCallback callback)
{
    { // for destructing lock_guard
        std::lock_guard<std::mutex> lock(_jmt);
        _pendingJob.push_back(JobDesc(tid, ms, callback));
    }
}

void BusyTimerImpl::run()
{
    _future = std::async(std::launch::async, [this] {
        MS elapsedMS = 0;
        do
        {
            auto startTime = system_clock::now();
            std::unique_lock<std::mutex> lock(_jmt);
            if(_jobs.empty())
            {
                _condvar.wait(lock, [this]{ return !_jobs.empty() || _shutdowned.load(std::memory_order_relaxed);} );
                if(_shutdowned.load(std::memory_order_relaxed))
                {
                    break;
                }
            }

            auto job = getSmallestDurationJob();
            if(job.remainer <= 0)
            {
                this->doJob(job);
            }
            else
            {
                _condvar.wait_for(lock, milliseconds(job.remainer));

                if(_shutdowned.load(std::memory_order_relaxed))
                {
                    break;
                }
                else if(duration_cast<milliseconds>(system_clock::now() - startTime).count() >= job.remainer)
                {
                    this->doJob(job);
                }
                elapsedMS = duration_cast<milliseconds>(system_clock::now() - startTime).count();
                if(elapsedMS > 0)
                {
                    this->reEvaluateJobs(elapsedMS);
                    elapsedMS = 0;
                }

                adoptPendingJobs();
            }
        } while(true);

        _shutdowned.store(true, std::memory_order_relaxed);
        _jobs.clear(); //for shutting down case
    });
}

bool BusyTimerImpl::jobRemained()
{
    std::lock_guard<std::mutex> lock(_jmt);
    return not _jobs.empty();
}

BusyTimerImpl::JobDesc BusyTimerImpl::getSmallestDurationJob()
{
    if(_jobs.empty())
    {
        return JobDesc(0, 0, nullptr);
    }
    else
    {
        return *_jobs.begin();
    }
}

void BusyTimerImpl::reEvaluateJobs(MS elapsed)
{
    for(auto& job : _jobs)
    {
        auto pJob = const_cast<JobDesc*>(&job);
        pJob->remainer -= elapsed;
    }
}

void BusyTimerImpl::doJob(BusyTimerImpl::JobDesc &job)
{
    _jobs.erase(job);
    job.callback(job.id);
}
}
