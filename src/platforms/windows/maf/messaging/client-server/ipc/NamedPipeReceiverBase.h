#pragma once
#define WIN32_LEAN_AND_MEAN
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
  NamedPipeReceiverBase() {}
  ~NamedPipeReceiverBase() { stop(); }
  bool init(Address address) {
    myaddr_ = std::move(address);
    pipeName_ = constructPipeName(myaddr_);
    return true;
  }

  bool start() {
    auto expectedRunning = false;
    if (running_.compare_exchange_strong(expectedRunning, true)) {
      startListening();
    }
    return true;
  }
  bool stop() {
    running_ = false;
    return false;
  }

  void deinit() {}

  bool running() const { return running_; }

  const Address &address() const { return myaddr_; }

 protected:
  virtual void startListening() {
    MAF_LOGGER_WARN(
        "listningThreadFunction must be overridden by derived class");
  }

  std::string pipeName_;
  Address myaddr_;
  std::atomic_bool running_ = false;
};

}  // namespace ipc
}  // namespace messaging
}  // namespace maf
