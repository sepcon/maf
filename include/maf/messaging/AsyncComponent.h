#pragma once

#include <future>

#include "Component.h"

namespace maf {
namespace messaging {

class AsyncComponent {
 public:
  using ThreadFunction = Component::ThreadFunction;
  using StoppedSignal = std::future<void>;

  AsyncComponent() = default;
  AsyncComponent(ComponentInstance comp) : instance_{std::move(comp)} {}

  AsyncComponent(AsyncComponent &&) = default;
  AsyncComponent &operator=(AsyncComponent &&) = default;

  const ComponentInstance &instance() const noexcept { return instance_; }

  static StoppedSignal run(const ComponentInstance &comp,
                           ThreadFunction threadInit = {},
                           ThreadFunction threadDeinit = {}) {
    return std::async(std::launch::async,
                      std::bind(&Component::run, comp, std::move(threadInit),
                                std::move(threadDeinit)));
  }

  void run(ThreadFunction threadInit = {}, ThreadFunction threadDeinit = {}) {
    if (instance_) {
      stopSignal_ =
          run(instance_, std::move(threadInit), std::move(threadDeinit));
    }
  }

  void stop() noexcept {
    if (instance_) {
      instance_->stop();
    }
  }

  void stopAndWait() noexcept {
      stop();

  }

  bool running() const noexcept { return stopSignal_.valid(); }

  void wait() noexcept {
    if (stopSignal_.valid()) {
      stopSignal_.get();
    }
  }

  template <class Rep, class Per>
  void waitFor(const std::chrono::duration<Rep, Per> &duration) noexcept {
    if (stopSignal_.valid()) {
      stopSignal_.wait_for(duration);
    }
  }

 private:
  ComponentInstance instance_;
  StoppedSignal stopSignal_;
};

}  // namespace messaging
}  // namespace maf
