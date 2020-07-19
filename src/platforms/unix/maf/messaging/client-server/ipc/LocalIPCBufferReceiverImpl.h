#pragma once

#include <maf/utils/serialization/Buffer.h>

#include <atomic>
#include <future>

#include "SocketShared.h"

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

using ByteArrayPtr = std::shared_ptr<srz::Buffer>;
using BytesComeCallback = std::function<void(srz::Buffer &&)>;

class LocalIPCBufferReceiverImpl {
 public:
  ~LocalIPCBufferReceiverImpl();
  bool init(const Address &addr);
  bool start();
  void stop();
  void deinit();
  bool running() const;
  const Address &address() const;
  void setObserver(BytesComeCallback callback);

 private:
  enum class State : char {
    Uninitialized,
    Initialized,
    Running,
    WaitingConnection,
    Stopped
  };

  class StoppedInterruption {};

  State getState() const { return state_.load(std::memory_order_acquire); }
  void setState(State state) { state_.store(state, std::memory_order_release); }

  bool waitAndProcessConnections();
  void interruptionPoint();

  BytesComeCallback bytesComeCallback_;
  Address myaddr_;
  sockaddr_un mySockAddr_;
  int fdMySock_;
  std::atomic<State> state_ = State::Uninitialized;
};

}  // namespace local
}  // namespace ipc
}  // namespace messaging
}  // namespace maf
