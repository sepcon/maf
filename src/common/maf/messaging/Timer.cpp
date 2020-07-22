#include <maf/logging/Logger.h>
#include <maf/messaging/Component.h>
#include <maf/messaging/Timer.h>

#include <cassert>
#include <memory>

#include "TimerManager.h"

namespace maf {

namespace messaging {

using JobID = TimerManager::JobID;
using TimerMgrPtr = std::shared_ptr<TimerManager>;

static std::shared_ptr<TimerManager> mgr_() {
  static auto mgr_ = std::make_shared<TimerManager>();
  return mgr_;
}

struct TimerDataPrv {
  TimerDataPrv(JobID _id, bool _cyclic) : id{_id}, cyclic{_cyclic} {}

  TimerMgrPtr mgr = mgr_();
  JobID id;
  bool cyclic;
};

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
      executor = this_component::getExecutor();
    }

    auto onTimeout = [executor = std::move(executor),
                      callback = std::move(callback), this] {
      executor->execute(std::move(callback));
      if (!d_->cyclic) {
        // mark that timer is not running anymore
        d_->id = TimerManager::invalidJobID();
      }
    };

    d_->id = d_->mgr->start(milliseconds.count(), onTimeout, d_->cyclic);
    MAF_LOGGER_INFO("Start new timer with id = ", d_->id);
  }
}

void Timer::restart() { d_->mgr->restart(d_->id); }

void Timer::stop() { d_->mgr->stop(d_->id); }

bool Timer::running() { return d_->mgr->isRunning(d_->id); }

void Timer::setCyclic(bool cyclic) {
  if (cyclic != d_->cyclic) {
    d_->cyclic = cyclic;
    d_->mgr->setCyclic(d_->id, cyclic);
  }
}

}  // namespace messaging
}  // namespace maf
