#pragma once

#include "SocketShared.h"
#include <atomic>
#include <future>
#include <maf/utils/serialization/ByteArray.h>

namespace maf {
namespace messaging {
namespace ipc {

using ByteArrayPtr = std::shared_ptr<srz::ByteArray>;
using BytesComeCallback = std::function<void(srz::ByteArray &&)>;

class LocalIPCReceiverImpl {
public:
  ~LocalIPCReceiverImpl();
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

} // namespace ipc
} // namespace messaging
} // namespace maf
