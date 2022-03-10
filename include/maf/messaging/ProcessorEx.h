#pragma once

#include "Processor.h"

namespace maf {
namespace messaging {

class ProcessorExBase {
 protected:
  ProcessorInstance instance_;

 public:
  ProcessorExBase(ProcessorID id = {}) {
    instance_ = Processor::create(std::move(id));
  }
  ProcessorExBase(ProcessorInstance inst) : instance_(std::move(inst)) {}
  Processor *operator->() noexcept { return instance_.get(); }
  const Processor *operator->() const noexcept { return instance_.get(); }
  Processor &operator*() noexcept { return *instance_; }
  const Processor &operator*() const noexcept { return *instance_; }
  ProcessorInstance instance() const { return instance_; }
};

class ProcessorEx : public ProcessorExBase {
 public:
  using ProcessorExBase::ProcessorExBase;
  virtual ~ProcessorEx() = default;
  ProcessorEx() = default;
  void run() {
    instance_->run([this] { threadInit(); }, [this] { threadDeinit(); });
  }

  void stop() { return instance_->stop(); }

 protected:
  virtual void threadInit() {}
  virtual void threadDeinit() {}
};

class AsyncProcessor : public ProcessorExBase {
 public:
  using ThreadFunction = Processor::ThreadFunction;
  using StoppedSignal = std::future<void>;
  using Timeout = std::chrono::milliseconds;
  using ProcessorExBase::ProcessorExBase;
  AsyncProcessor(AsyncProcessor &&) = default;
  AsyncProcessor &operator=(AsyncProcessor &&) = default;
  ~AsyncProcessor() { stopAndWait(); }

  static StoppedSignal launchProcessor(const ProcessorInstance &comp,
                                       ThreadFunction threadInit = {},
                                       ThreadFunction threadDeinit = {}) {
    return std::async(std::launch::async,
                      std::bind(&Processor::run, comp, std::move(threadInit),
                                std::move(threadDeinit)));
  }

  void launch(ThreadFunction threadInit = {},
              ThreadFunction threadDeinit = {}) {
    if (instance_ && !running()) {
      stopSignal_ = launchProcessor(instance_, std::move(threadInit),
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
