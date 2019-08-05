#pragma once

#include "maf/threading/TimerManager.h"
#include "maf/utils/cppextension/SyncObject.h"
#include <chrono>
#include <set>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <future>


namespace maf {
namespace threading {

struct JobDesc;
struct TimerManagerImpl
{
    using JobID = TimerManager::JobID;
    using Duration = TimerManager::Duration;
    using TimeOutCallback = std::function<void(JobID, bool)>;
    static constexpr size_t MAX_JOBS_COUNT = 1000;

    TimerManagerImpl() : _shutdowned(true), _workerThreadIsRunning(false) {}
    ~TimerManagerImpl();
    bool start(TimerManagerImpl::JobID jid, Duration ms, TimeOutCallback callback, bool cyclic);
    void restart(JobID jid);
    void stop(JobID jid);
    bool isRunning(JobID jid);
    void setCyclic(JobID jid, bool cyclic);
    void shutdown();

private:
    using JobDescRef = std::shared_ptr<JobDesc>;
    using JobsContainer = nstl::SyncObject<std::vector<JobDescRef>>;
    using JobsIterator = JobsContainer::DataType::iterator;
    using JobsCIterator = JobsContainer::DataType::const_iterator;
    friend struct JobComp;
    friend struct JobDesc;

    size_t jobsCount();
    void run() noexcept;
    void startImmediately(JobID jid, Duration ms, TimeOutCallback callback, bool cyclic) noexcept;
    void storePendingJob(JobID jid, Duration ms, TimeOutCallback callback, bool cyclic) noexcept;
    void storePendingJob(JobDescRef job) noexcept;
    void doJob__(JobDescRef job);
    void adoptPendingJobs__() noexcept;
    static JobDescRef getShorttestDurationJob__(const JobsContainer &jobs) noexcept;
    void reorderRunningJobs__() noexcept;
    JobDescRef scheduleShorttestJob__() noexcept;
    bool runJobIfExpired__(JobDescRef job);
    void cleanup();

    static std::unique_ptr<std::lock_guard<std::mutex> > autolock(TimerManagerImpl::JobsContainer& jobs, bool sync = true);
    static void addOrReplace(JobsContainer& jobs, JobDescRef newJob, bool sync = true);
    static JobDescRef removeAndInvalidateJob(JobsContainer &jobs, JobID jid);
    static JobsIterator findJob__(JobsContainer &jobs, JobID jid);

    std::future<void> _future;
    JobsContainer _runningJobs;
    JobsContainer _pendingJobs;
    std::mutex _jmt;
    std::condition_variable_any _condvar;
    std::atomic_bool _shutdowned;
    std::atomic_bool _workerThreadIsRunning;
};
}
}
