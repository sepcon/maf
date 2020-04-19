#pragma once

#include "PipeShared.h"
#include <Windows.h>
#include <atomic>
#include <maf/logging/Logger.h>
#include <string>
#include <thread>

namespace maf {

namespace messaging {
namespace ipc {

class NamedPipeReceiverBase {
public:
  NamedPipeReceiverBase() : stopped_(true) {}
  ~NamedPipeReceiverBase() {
    if (!stopped_) {
      stopListening();
    }

    if (workerThread_.joinable()) {
      workerThread_.join();
    }
  }
  bool initConnection(const Address &address, bool isClientMode = false) {
    isClient_ = isClientMode;
    if (isClientMode) {
      static std::atomic<uint16_t> receiverCount(0);
      receiverCount += 1;
      uint16_t randomPort = receiverCount;
      myaddr_ =
          Address(address.get_name() + std::to_string(GetCurrentProcessId()),
                  randomPort);
    } else {
      myaddr_ = address;
    }
    pipeName_ = constructPipeName(myaddr_);
    return true;
  }

  bool startListening() {
    if (!listening()) {
      stopped_.store(false, std::memory_order_release);
      workerThread_ =
          std::thread{&NamedPipeReceiverBase::listningThreadFunction, this};
    }
    return true;
  }
  bool stopListening() {
    stopped_.store(true, std::memory_order_release);
    waitForWorkerThreadToStop();
    return false;
  }
  bool listening() const { return !stopped_.load(std::memory_order_acquire); }
  const Address &address() const { return myaddr_; }

protected:
  virtual void listningThreadFunction() {
    MAF_LOGGER_WARN(
        "listningThreadFunction must be overridden by derived class");
  }
  void waitForWorkerThreadToStop() {
    if (workerThread_.joinable()) {
      workerThread_.join();
    }
  }
  std::string pipeName_;
  std::thread workerThread_;
  Address myaddr_;
  std::atomic_bool stopped_;
  bool isClient_ = false;
};

} // namespace ipc
} // namespace messaging
} // namespace maf
