#include "TimerManagerImpl.h"
#include <algorithm>
#include <maf/logging/Logger.h>

using namespace std::chrono;

namespace maf {

namespace messaging {

static bool test(const std::atomic_bool &flag) {
  return flag.load(std::memory_order_acquire);
}

static void set(std::atomic_bool &flag, bool value) {
  return flag.store(value, std::memory_order_release);
}

template <typename Lockable> class ConcurrentMutex {
  Lockable &l1;
  Lockable &l2_;

public:
  ConcurrentMutex(Lockable &l1, Lockable &l2) : l1(l1), l2_(l2) {}
  void lock() { return std::lock(l1, l2_); }
  void unlock() {
    l1.unlock();
    l2_.unlock();
  }
};

struct JobDesc {
  using JobID = TimerManagerImpl::JobID;
  using Duration = TimerManagerImpl::Duration;
  using TimeOutCallback = TimerManagerImpl::TimeOutCallback;

  JobDesc(JobID id_, Duration duration_, TimeOutCallback callback_,
          bool isCyclic_ = false) {
    *d_ = D{id_, duration_, std::move(callback_), isCyclic_};
  }
  Duration remainTime() {
    std::lock_guard lock(d_);
    return d_->duration - elapseTime();
  }

  void reset() { d_.atomic()->startTime = system_clock::now(); }
  bool expired() {
    std::lock_guard lock(d_);
    return elapseTime() >= d_->duration;
  }
  void invalidate() { d_.atomic()->callback = {}; }
  bool valid() const { return bool(d_.atomic()->callback); }
  Duration duration() const { return d_.atomic()->duration; }
  JobID id() const { return d_->id; }
  bool isCyclic() const { return d_.atomic()->isCyclic; }
  void setCyclic(bool value) { d_.atomic()->isCyclic = value; }
  TimeOutCallback callback() { return d_.atomic()->callback; }

private:
  Duration elapseTime() {
    return duration_cast<milliseconds>(system_clock::now() - d_->startTime)
        .count();
  }
  struct D {
    D() = default;
    D(JobID id_, Duration duration_, TimeOutCallback callback_,
      bool isCyclic_ = false)
        : id(id_), startTime(system_clock::now()), duration(duration_),
          callback(callback_), isCyclic(isCyclic_) {}

    JobID id;
    system_clock::time_point startTime;
    Duration duration;
    TimeOutCallback callback;
    bool isCyclic;
  };

  threading::Lockable<D> d_;
};

struct JobComp {
  bool operator()(const TimerManagerImpl::JobDescRef &lhs,
                  const TimerManagerImpl::JobDescRef &rhs) {
    return lhs->remainTime() > rhs->remainTime();
  }
};

TimerManagerImpl::~TimerManagerImpl() {
  if (thread_.joinable()) {
    thread_.join();
  }
}

bool TimerManagerImpl::start(JobID jid, Duration ms, TimeOutCallback callback,
                             bool cyclic) {
  bool success = true;

  if (!test(workerThreadIsRunning_)) {
    startImmediately(jid, ms, callback, cyclic);
  } else {
    if (jobsCount() < MAX_JOBS_COUNT) {
      storePendingJob(jid, ms, callback, cyclic);
      condvar_.notify_one();
    } else {
      MAF_LOGGER_WARN("Cannot start timer for job ", jid,
                      " Due to number of jobs exceeds the permited amount(",
                      MAX_JOBS_COUNT, "), please try with other BusyTimer");
      success = false;
    }
  }

  if (success && test(shutdowned_)) {
    set(shutdowned_, false);
    run();
  }
  return success;
}

void TimerManagerImpl::restart(TimerManagerImpl::JobID jid) {
  if (!test(shutdowned_)) {
    std::unique_lock<JobsContainer> jobslock(runningJobs_);
    auto itJob = findJob__(runningJobs_, jid);
    if (itJob != runningJobs_->end()) {
      MAF_LOGGER_INFO("Timer ", jid,
                      " is restarted with duration = ", (*itJob)->duration());

      (*itJob)->reset();
      jobslock.unlock();
      condvar_.notify_one();
    }
  }
}

void TimerManagerImpl::stop(TimerManagerImpl::JobID jid) {
  if (!test(shutdowned_)) {
    if (removeAndInvalidateJob(pendingJobs_, jid)) {
      MAF_LOGGER_INFO("Job ", jid, " is canceled");
    } else if (removeAndInvalidateJob(runningJobs_, jid)) {
      MAF_LOGGER_INFO("Job ", jid, " is canceled");
      condvar_.notify_one();
    } else {
      MAF_LOGGER_WARN("Job ", jid, " does not exist or is already canceled");
    }
  }
}

bool TimerManagerImpl::isRunning(TimerManagerImpl::JobID jid) {
  bool running = false;
  if (!test(shutdowned_)) {
    auto jobFoundOn = [&jid](const JobsContainer &jobs) -> bool {
      std::lock_guard lock(jobs);
      for (auto &job : *jobs) {
        if (job->id() == jid) {
          return true;
        }
      }
      return false;
    };

    running = (jobFoundOn(pendingJobs_) || jobFoundOn(runningJobs_));
  }

  return running;
}

void TimerManagerImpl::setCyclic(TimerManagerImpl::JobID jid, bool cyclic) {
  if (!test(shutdowned_)) {
    auto setCyclic = [](JobsContainer &jobs, JobID jid, bool value) -> bool {
      std::lock_guard lock(jobs);
      auto itJob = findJob__(jobs, jid);
      if (itJob != jobs->end()) {
        (*itJob)->setCyclic(value);
      }
      return itJob != jobs->end();
    };

    if (!setCyclic(pendingJobs_, jid, cyclic)) {
      setCyclic(runningJobs_, jid, cyclic);
    }
  }
}

void TimerManagerImpl::stop() {
  set(shutdowned_, true);
  condvar_.notify_one();
}

size_t TimerManagerImpl::jobsCount() {
  std::scoped_lock lock(pendingJobs_, runningJobs_);
  return pendingJobs_->size() + runningJobs_->size();
}

void TimerManagerImpl::startImmediately(JobID jid, Duration ms,
                                        TimeOutCallback callback,
                                        bool cyclic) noexcept {
  addOrReplace(runningJobs_,
               std::make_shared<JobDesc>(jid, ms, callback, cyclic));
}

void TimerManagerImpl::reorderRunningJobs__() noexcept {
  std::make_heap(runningJobs_->begin(), runningJobs_->end(), JobComp());
}

void TimerManagerImpl::adoptPendingJobs__() noexcept {
  if (!pendingJobs_->empty()) {
    for (auto &job : *pendingJobs_) {
      addOrReplace(runningJobs_, std::move(job), false);
    }
    pendingJobs_->clear();
  }
}

void TimerManagerImpl::storePendingJob(JobID jid, Duration ms,
                                       TimeOutCallback callback,
                                       bool cyclic) noexcept {
  storePendingJob(std::make_shared<JobDesc>(jid, ms, callback, cyclic));
}

void TimerManagerImpl::storePendingJob(JobDescRef job) noexcept {
  addOrReplace(pendingJobs_, std::move(job));
}

TimerManagerImpl::JobDescRef
TimerManagerImpl::removeAndInvalidateJob(JobsContainer &jobs, JobID jid) {
  JobDescRef job;
  std::lock_guard lock(jobs);
  for (auto &j : *jobs) {
    if (j->id() == jid) {
      job = std::move(j);
      job->invalidate();

      std::swap(j, jobs->back());
      jobs->pop_back();

      break;
    }
  }
  return job;
}

TimerManagerImpl::JobsIterator TimerManagerImpl::findJob__(JobsContainer &jobs,
                                                           JobID jid) {
  for (auto itJob = jobs->begin(); itJob != jobs->end(); ++itJob) {
    if ((*itJob)->id() == jid) {
      return itJob;
    }
  }
  return jobs->end();
}

bool TimerManagerImpl::runJobIfExpired__(TimerManagerImpl::JobDescRef job) {
  bool executed = false;
  if (job->expired()) {
    try {
      // for case of job has been stopped when waiting to be expired
      if (job->valid()) {
        this->doJob__(job);
      }
    } catch (const std::exception &e) {
      MAF_LOGGER_INFO("Catch exception when executing job's callback: ",
                      e.what());
    } catch (...) {
      MAF_LOGGER_INFO(
          "Uncaught exception occurred when executing job's callback");
    }
    executed = true;
  }
  return executed;
}

void TimerManagerImpl::run() noexcept {
  thread_ = std::thread([this] {
    set(workerThreadIsRunning_, true);
    MAF_LOGGER_INFO("TimerManager thread is running!");
    do {
      ConcurrentMutex<JobsContainer> ccMutex(pendingJobs_, runningJobs_);
      std::unique_lock<decltype(ccMutex)> jobsLock(ccMutex);

      this->reorderRunningJobs__();
      auto job = getShorttestDurationJob__(runningJobs_);

      if (job) {
        if (runJobIfExpired__(job)) {
          this->scheduleShorttestJob__();
        } else {
          if (test(shutdowned_)) {
            break;
          }
          // TBD: _shutdowned might be set here
          condvar_.wait_for(jobsLock, milliseconds(job->remainTime()));

          if (test(shutdowned_)) {
            break;
          }

          if (runJobIfExpired__(job)) {
            this->scheduleShorttestJob__();
          }
        }
      } else // there's no job to do
      {
        condvar_.wait(ccMutex, [this] {
          return (this->pendingJobs_->size() > 0) ||
                 (test(shutdowned_) == true);
        });
      }

      this->adoptPendingJobs__();

    } while (!test(shutdowned_));
    this->cleanup();
  });
}

TimerManagerImpl::JobDescRef TimerManagerImpl::getShorttestDurationJob__(
    const JobsContainer &jobs) noexcept {
  if (jobs->empty()) {
    return nullptr;
  } else {
    return jobs->front();
  }
}

void TimerManagerImpl::doJob__(TimerManagerImpl::JobDescRef job) {
  job->callback()(job->id(), job->isCyclic());
}

TimerManagerImpl::JobDescRef
TimerManagerImpl::scheduleShorttestJob__() noexcept {
  JobDescRef job;
  if (!runningJobs_->empty()) {
    std::pop_heap(runningJobs_->begin(), runningJobs_->end(), JobComp());

    if (runningJobs_->back()->isCyclic()) {
      runningJobs_->back()->reset();
      job = runningJobs_->back();
      std::push_heap(runningJobs_->begin(), runningJobs_->end());
    } else // Remove job when it expired
    {
      job = std::move(runningJobs_->back());
      runningJobs_->pop_back();
    }
  }
  return job;
}

void TimerManagerImpl::cleanup() {

  std::scoped_lock(pendingJobs_, runningJobs_);
  set(workerThreadIsRunning_, false);
  if (test(shutdowned_)) {
    pendingJobs_->clear();
    runningJobs_->clear();
  } else {
    // due to the worker thread is no longer running, then _shutdowned flag must
    // be set to launch new worker thread later if client code invoke start
    // function again
    set(shutdowned_, true);
  }
}

void TimerManagerImpl::addOrReplace(TimerManagerImpl::JobsContainer &jobs,
                                    TimerManagerImpl::JobDescRef newJob,
                                    bool sync) {
  if (newJob) {
    std::unique_ptr<std::lock_guard<JobsContainer>> lock;
    if (sync) {
      lock = std::make_unique<std::lock_guard<JobsContainer>>(jobs);
    }

    auto itJob = findJob__(jobs, newJob->id());
    if (itJob != jobs->end()) {
      *itJob = std::move(newJob);
    } else {
      jobs->emplace_back(std::move(newJob));
    }
  }
}

} // namespace messaging
} // namespace maf
