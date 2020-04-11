#include <atomic>
#include <cassert>
#include <condition_variable>
#include <maf/logging/Logger.h>
#include <maf/messaging/SyncCallbackExecutor.h>
#include <maf/messaging/SyncTimer.h>
#include <mutex>

namespace maf {

namespace messaging {

struct SyncTimerDataPrv {
  std::condition_variable_any timeoutCondition;
  std::recursive_mutex mutex;
  bool running = false;
  bool cyclic = false;
};

SyncTimer::SyncTimer(bool cyclic)
    : d_{new SyncTimerDataPrv{{}, {}, false, cyclic}} {}

SyncTimer::~SyncTimer() {
  stop();
  delete d_;
}
void SyncTimer::start(long long milliseconds, TimeOutCallback callback,
                      ExecutorPtr executor) {
  start(std::chrono::milliseconds{milliseconds}, std::move(callback),
        std::move(executor));
}

void SyncTimer::start(std::chrono::milliseconds milliseconds,
                      TimeOutCallback callback, ExecutorPtr executor) {
  if (!callback) {
    MAF_LOGGER_ERROR("[TimerImpl]: Please specify not null callback");
  } else {
    if (running()) {
      MAF_LOGGER_INFO("TimerImpl is still running, then stop!");
      stop();
    }

    std::unique_lock lock(d_->mutex);
    d_->running = true;

    do {
      if (!d_->timeoutCondition.wait_for(lock, milliseconds,
                                         [this] { return !d_->running; })) {
        if (executor) {
          executor->execute(d_->cyclic ? callback : std::move(callback));
        } else {
          callback();
        }
      }

      if (!d_->running) // restarted
      {
        break;
      }

    } while (d_->cyclic);

    d_->running = false;
  }
}

void SyncTimer::restart() { d_->timeoutCondition.notify_one(); }

void SyncTimer::stop() {
  std::lock_guard lock(d_->mutex);
  if (d_->running) {
    d_->running = false;
    d_->timeoutCondition.notify_one();
  }
}

bool SyncTimer::running() { return d_->running; }

void SyncTimer::setCyclic(bool cyclic) {
  std::lock_guard lock(d_->mutex);
  d_->cyclic = cyclic;
}

} // namespace messaging
} // namespace maf
