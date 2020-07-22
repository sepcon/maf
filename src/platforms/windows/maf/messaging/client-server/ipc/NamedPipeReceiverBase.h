#pragma once

#include <Windows.h>
#include <maf/logging/Logger.h>

#include <atomic>
#include <string>
#include <thread>

#include "PipeShared.h"

namespace maf {

namespace messaging {
namespace ipc {

class NamedPipeReceiverBase {
 public:
  NamedPipeReceiverBase() : stopped_(true) {}
  ~NamedPipeReceiverBase() {
    if (!stopped_) {
      stop();
    }
  }
  bool init(Address address) {
    myaddr_ = std::move(address);
    pipeName_ = constructPipeName(myaddr_);
    return true;
  }

  bool start() {
    if (!running()) {
      stopped_.store(false, std::memory_order_release);
      startListening();
    }
    return true;
  }
  bool stop() {
    stopped_.store(true, std::memory_order_release);
    return false;
  }

  void deinit() {}

  bool running() const { return !stopped_.load(std::memory_order_acquire); }

  const Address &address() const { return myaddr_; }

 protected:
  virtual void startListening() {
    MAF_LOGGER_WARN(
        "listningThreadFunction must be overridden by derived class");
  }

  std::string pipeName_;
  Address myaddr_;
  std::atomic_bool stopped_;
};

}  // namespace ipc
}  // namespace messaging
}  // namespace maf
