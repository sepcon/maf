#include "TimerManager.h"
#include <cassert>
#include <maf/logging/Logger.h>
#include <maf/messaging/AsyncCallbackExecutor.h>
#include <maf/messaging/BasicMessages.h>
#include <maf/messaging/Timer.h>

namespace maf {

namespace messaging {

using JobID = TimerManager::JobID;

struct TimerDataPrv {
  TimerDataPrv(JobID _id, bool _cyclic) : id{_id}, cyclic{_cyclic} {}

  JobID id;
  bool cyclic;
};

static TimerManager &ourMgr() {
  static TimerManager mgr;
  return mgr;
}

Timer::Timer(bool cyclic)
    : d_{new TimerDataPrv{TimerManager::invalidJobID(), cyclic}} {}

Timer::~Timer() { stop(); }
void Timer::start(long long milliseconds, TimeOutCallback callback,
                  ExecutorPtr executor) {
  start(std::chrono::milliseconds{milliseconds}, std::move(callback),
        std::move(executor));
}

void Timer::start(std::chrono::milliseconds milliseconds,
                  TimeOutCallback callback, ExecutorPtr executor) {
  if (!callback) {
    MAF_LOGGER_ERROR("[TimerImpl]: Please specify not null callback");
  } else {
    if (running()) {
      MAF_LOGGER_INFO("TimerImpl is still running, then stop!");
      stop();
    }

    if (!executor) {
      executor =
          std::make_shared<AsyncCallbackExecutor>(RunningComponent::weak());
    }
    auto onTimeout = [executor = std::move(executor),
                      callback = std::move(callback), this] {
      executor->execute(std::move(callback));
      if (!d_->cyclic) {
        // mark that timer is not running anymore
        d_->id = TimerManager::invalidJobID();
      }
    };

    d_->id = ourMgr().start(milliseconds.count(), onTimeout, d_->cyclic);
    MAF_LOGGER_INFO("Start new timer with id = ", d_->id);
  }
}

void Timer::restart() { ourMgr().restart(d_->id); }

void Timer::stop() { ourMgr().stop(d_->id); }

bool Timer::running() { return ourMgr().isRunning(d_->id); }

void Timer::setCyclic(bool cyclic) {
  if (cyclic != d_->cyclic) {
    d_->cyclic = cyclic;
    ourMgr().setCyclic(d_->id, cyclic);
  }
}

} // namespace messaging
} // namespace maf
