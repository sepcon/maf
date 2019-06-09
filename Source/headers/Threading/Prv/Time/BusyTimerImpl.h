#ifndef BUSY_TIMER_IMPL_H
#define BUSY_TIMER_IMPL_H

#include "headers/Threading/Interfaces/BusyTimer.h"
#include <chrono>
#include <set>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <future>


namespace thaf {
namespace Threading {

struct JobDesc;
struct BusyTimerImpl
{
    using JobID = BusyTimer::JobID;
    using Duration = BusyTimer::Duration;
    using TimeOutCallback = std::function<void(JobID, bool)>;
    const size_t MAX_JOBS_COUNT = 1000;

    BusyTimerImpl() : _shutdowned(true), _workerThreadIsRunning(false) {}
	~BusyTimerImpl();
    bool start(BusyTimerImpl::JobID jid, Duration ms, TimeOutCallback callback, bool cyclic);
    void restart(JobID jid);
    void stop(JobID jid);
    bool isRunning(JobID jid);
    void setRecyclic(JobID jid, bool cyclic);
    void shutdown();

private:
    using JobDescRef = std::shared_ptr<JobDesc>;
    using JobsContainer = std::vector<JobDescRef>;

    friend struct JobComp;
    friend struct JobDesc;

    void run();
    void startJob(JobID jid, Duration ms, TimeOutCallback callback, bool cyclic);
    void reorderRunningJobs();
    void startPendingJobs();
    void storePendingJob(JobID jid, Duration ms, TimeOutCallback callback, bool cyclic);
    void storePendingJob(JobDescRef job);
    JobDescRef getShorttestDurationJob();
    void doJob(JobDescRef job);
    JobDescRef rescheduleShorttestJob();
    void cleanup();

    static void addOrReplace(JobsContainer& jobs, JobDescRef newJob);
    static JobDescRef removeJob(JobsContainer &jobs, JobID jid);
    static JobsContainer::iterator findJob(JobsContainer &jobs, JobID jid);
    static JobDescRef getJob(JobsContainer& jobs, JobID jid);

    std::future<void> _future;
    JobsContainer _runningJobs;
    JobsContainer _pendingJobs;
    std::mutex _jmt;
    std::condition_variable _condvar;
    std::atomic_bool _shutdowned;
    std::atomic_bool _workerThreadIsRunning;
};
}
}
#endif
