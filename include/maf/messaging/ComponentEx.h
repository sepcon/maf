#pragma once

#include "Component.h"

namespace maf {
namespace messaging {

class ComponentExBase {
 protected:
  ComponentInstance instance_;

 public:
  ComponentExBase(ComponentID id = {}) {
    instance_ = Component::create(std::move(id));
  }
  ComponentExBase(ComponentInstance inst) : instance_(std::move(inst)) {}
  Component *operator->() noexcept { return instance_.get(); }
  const Component *operator->() const noexcept { return instance_.get(); }
  Component &operator*() noexcept { return *instance_; }
  const Component &operator*() const noexcept { return *instance_; }
  ComponentInstance instance() const { return instance_; }
};

class ComponentEx : public ComponentExBase {
 public:
  using ComponentExBase::ComponentExBase;
  ComponentEx() = default;
  void run() {
    instance_->run([this] { threadInit(); }, [this] { threadDeinit(); });
  }

  void stop() { return instance_->stop(); }

 protected:
  virtual void threadInit() {}
  virtual void threadDeinit() {}
};

class AsyncComponent : public ComponentExBase {
 public:
  using ThreadFunction = Component::ThreadFunction;
  using StoppedSignal = std::future<void>;
  using Timeout = std::chrono::milliseconds;
  using ComponentExBase::ComponentExBase;
  AsyncComponent(AsyncComponent &&) = default;
  AsyncComponent &operator=(AsyncComponent &&) = default;
  ~AsyncComponent() { stopAndWait(); }

  static StoppedSignal launchComponent(const ComponentInstance &comp,
                                       ThreadFunction threadInit = {},
                                       ThreadFunction threadDeinit = {}) {
    return std::async(std::launch::async,
                      std::bind(&Component::run, comp, std::move(threadInit),
                                std::move(threadDeinit)));
  }

  void launch(ThreadFunction threadInit = {},
              ThreadFunction threadDeinit = {}) {
    if (instance_ && !running()) {
      stopSignal_ = launchComponent(instance_, std::move(threadInit),
                                    std::move(threadDeinit));
    }
  }

  void stopAndWait(const Timeout &duration = Timeout{0}) {
    if (running()) {
      (*this)->stop();
      wait(duration);
    }
  }

  bool running() const noexcept { return stopSignal_.valid(); }

  void wait(const Timeout &duration = Timeout{0}) {
    if (stopSignal_.valid()) {
      if (duration > Timeout{0}) {
        if (stopSignal_.wait_for(duration) == std::future_status::ready) {
          stopSignal_.get();
        }
      } else {
        stopSignal_.get();
      }
    }
  }

 private:
  StoppedSignal stopSignal_;
};

}  // namespace messaging
}  // namespace maf
