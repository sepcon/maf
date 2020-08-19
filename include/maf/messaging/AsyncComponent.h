#pragma once

#include <future>

#include "Component.h"

namespace maf {
namespace messaging {

class AsyncComponent {
 public:
  using ThreadFunction = Component::ThreadFunction;
  using StoppedSignal = std::future<void>;
  using Timeout = std::chrono::milliseconds;

  AsyncComponent() = default;
  AsyncComponent(ComponentInstance comp) : instance_{std::move(comp)} {}

  AsyncComponent(AsyncComponent &&) = default;
  AsyncComponent &operator=(AsyncComponent &&) = default;

  const ComponentInstance &instance() const noexcept { return instance_; }

  static StoppedSignal runAsync(const ComponentInstance &comp,
                                ThreadFunction threadInit = {},
                                ThreadFunction threadDeinit = {}) {
    return std::async(std::launch::async,
                      std::bind(&Component::run, comp, std::move(threadInit),
                                std::move(threadDeinit)));
  }

  void run(ThreadFunction threadInit = {}, ThreadFunction threadDeinit = {}) {
    if (instance_ && !running()) {
      stopSignal_ =
          runAsync(instance_, std::move(threadInit), std::move(threadDeinit));
    }
  }

  void stop() {
    if (instance_) {
      instance_->stop();
    }
  }

  void stopAndWait(const Timeout &duration = Timeout{0}) {
    stop();
    wait(duration);
  }

  bool running() const noexcept { return stopSignal_.valid(); }

  void wait(const Timeout &duration = Timeout{0}) {
    if (stopSignal_.valid()) {
      if (duration > Timeout{0}) {
        stopSignal_.wait_for(duration);
      } else {
        stopSignal_.get();
      }
    }
  }

 private:
  ComponentInstance instance_;
  StoppedSignal stopSignal_;
};

}  // namespace messaging
}  // namespace maf
