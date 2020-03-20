#pragma once

#include "SocketShared.h"
#include <future>

namespace maf {
namespace messaging {
namespace ipc {

using ByteArrayPtr = std::shared_ptr<srz::ByteArray>;
using BytesComeCallback = std::function<void(const ByteArrayPtr &)>;

class LocalIPCReceiverImpl {
public:
  ~LocalIPCReceiverImpl();
  bool initConnection(const Address &addr, bool isClientMode = false);
  bool startListening();
  bool stopListening();
  bool listening() const;
  const Address &address() const;
  void registerObserver(BytesComeCallback callback);

private:
  bool listeningThreadFunc(int fdMySock);

  BytesComeCallback bytesComeCallback_;
  Address myaddr_;
  sockaddr_un mySockAddr_;
  std::thread listeningThread_;
  std::atomic_bool stopped_ = false;
  std::atomic_bool selecting_ = false;
};

} // namespace ipc
} // namespace messaging
} // namespace maf
