#pragma once

#include "TimerManager.h"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <maf/threading/Lockable.h>
#include <memory>
#include <thread>
#include <vector>

namespace maf {
namespace messaging {

struct JobDesc;
struct TimerManagerImpl {
  using JobID = TimerManager::JobID;
  using Duration = TimerManager::Duration;
  using TimeOutCallback = std::function<void(JobID, bool)>;
  static constexpr size_t MAX_JOBS_COUNT = 1000;

  TimerManagerImpl() : shutdowned_(true), workerThreadIsRunning_(false) {}
  ~TimerManagerImpl();
  bool start(JobID jid, Duration ms, TimeOutCallback callback, bool cyclic);
  void restart(JobID jid);
  void stop(JobID jid);
  bool isRunning(JobID jid);
  void setCyclic(JobID jid, bool cyclic);
  void stop();

private:
  using JobDescRef = std::shared_ptr<JobDesc>;
  using JobsContainer = threading::Lockable<std::vector<JobDescRef>>;
  using JobsIterator = JobsContainer::DataType::iterator;
  using JobsCIterator = JobsContainer::DataType::const_iterator;
  friend struct JobComp;
  friend struct JobDesc;

  size_t jobsCount();
  void run() noexcept;
  void startImmediately(JobID jid, Duration ms, TimeOutCallback callback,
                        bool cyclic) noexcept;

  void storePendingJob(JobID jid, Duration ms, TimeOutCallback callback,
                       bool cyclic) noexcept;

  void storePendingJob(JobDescRef job) noexcept;
  void doJob__(JobDescRef job);
  void adoptPendingJobs__() noexcept;
  static JobDescRef
  getShorttestDurationJob__(const JobsContainer &jobs) noexcept;
  void reorderRunningJobs__() noexcept;
  JobDescRef scheduleShorttestJob__() noexcept;
  bool runJobIfExpired__(JobDescRef job);
  void cleanup();

  static std::lock_guard<JobsContainer> autolock(JobsContainer &jobs,
                                                 bool sync = true);
  static void addOrReplace(JobsContainer &jobs, JobDescRef newJob,
                           bool sync = true);
  static JobDescRef removeAndInvalidateJob(JobsContainer &jobs, JobID jid);
  static JobsIterator findJob__(JobsContainer &jobs, JobID jid);

  std::thread thread_;
  JobsContainer runningJobs_;
  JobsContainer pendingJobs_;
  std::mutex jmt_;
  std::condition_variable_any condvar_;
  std::atomic_bool shutdowned_;
  std::atomic_bool workerThreadIsRunning_;
};
} // namespace messaging
} // namespace maf
