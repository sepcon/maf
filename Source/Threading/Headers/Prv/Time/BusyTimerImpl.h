#ifndef BUSY_TIMER_IMPL_H
#define BUSY_TIMER_IMPL_H

#include "Interfaces/BusyTimer.h"
#include <set>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <future>

namespace Threading {

struct BusyTimerImpl
{
    using TimerID = BusyTimer::TimerID;
    using MS = BusyTimer::MS;
    using TimeOutCallback = BusyTimer::TimeOutCallback;

    BusyTimerImpl() : _shutdowned(true){}
    void start(TimerID tid, MS ms, TimeOutCallback callback);
    void stop(TimerID tid);
    bool isRunning(TimerID tid);
    void shutdown();

private:
    struct JobDesc
    {
        JobDesc(TimerID id_, MS remainer_, TimeOutCallback callback_)
            : id(id_), remainer(remainer_), callback(callback_){}

        TimerID id;
        MS remainer;
        TimeOutCallback callback;
    };

    struct JobComp
    {
        bool operator()(const JobDesc& lhs,
            const JobDesc& rhs) {
            return lhs.remainer < rhs.remainer;
        }
    };

    void adoptJob(TimerID tid, MS ms, TimeOutCallback callback);
    void adoptPendingJobs();
    void storeJob(TimerID tid, MS ms, TimeOutCallback callback);
    void run();
    bool jobRemained();
    JobDesc getSmallestDurationJob();
    void reEvaluateJobs(MS elapsed);
    void doJob(JobDesc& job);

    std::future<void> _future;
    std::multiset<JobDesc, JobComp> _jobs;
    std::vector<JobDesc> _pendingJob;
    std::mutex _jmt;
    std::condition_variable _condvar;
    std::atomic_bool _shutdowned;
};
}

#endif
