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
    using JobID = BusyTimer::JobID;
    using Duration = BusyTimer::Duration;
    using TimeOutCallback = BusyTimer::TimeOutCallback;
    const size_t MAX_JOBS_COUNT = 1000;

    BusyTimerImpl() : _shutdowned(true), _workerThreadIsRunning(false) {}
	~BusyTimerImpl();
    bool start(JobID tid, Duration ms, TimeOutCallback callback);
    void restart(JobID tid);
    void stop(JobID tid);
    bool isRunning(JobID tid);
    void shutdown();

private:

    struct JobDesc
    {
        JobDesc(JobID id_, Duration duration_, TimeOutCallback callback_)
            : id(id_), duration(duration_), remainer(duration_), callback(callback_){}
        void reset() { remainer = duration; }
        JobID id;
        Duration duration;
        Duration remainer;
        TimeOutCallback callback;
    };

    struct JobComp
    {
        bool operator()(const JobDesc& lhs,
            const JobDesc& rhs) {
            return lhs.remainer < rhs.remainer;
        }
    };

    using JobDescRef = std::shared_ptr<JobDesc>;
    using JobsContainer = std::vector<JobDescRef>;
    void run();
    void startJob(JobID tid, Duration ms, TimeOutCallback callback);
    void startPendingJobs();
    void storePendingJob(JobID tid, Duration ms, TimeOutCallback callback);
    void storePendingJob(JobDescRef job);
    bool jobRemained();
    JobDescRef getShorttestDurationJob();
    void reEvaluateJobs(Duration elapsed);
    void doJob(JobDescRef job);
    void cleanup();

    static void addOrReplace(JobsContainer& jobs, JobDescRef newJob);
	static JobDescRef removeJob(JobsContainer &jobs, JobID jid);
	static JobsContainer::iterator findJob(JobsContainer &jobs, JobID jid);

    std::future<void> _future;
    JobsContainer _runningJobs;
    JobsContainer _pendingJobs;
    std::mutex _jmt;
    std::condition_variable _condvar;
    std::atomic_bool _shutdowned;
    std::atomic_bool _workerThreadIsRunning;
};
}

#endif
