#pragma once

#include <set>
#include <thread>

#include "../ServerBase.h"
#include "BufferReceiverIF.h"
#include "IPCTypes.h"

namespace maf {
namespace messaging {
namespace ipc {

class BufferSenderIF;
class BufferReceiverIF;

namespace local {

class LocalIPCServer : public ServerBase, public BytesComeObserver {
 public:
  LocalIPCServer();
  ~LocalIPCServer() override;
  bool init(const Address &serverAddress) override;
  bool start() override;
  void stop() override;

  ActionCallStatus sendMessageToClient(const CSMessagePtr &msg,
                                       const Address &addr) override;
  void notifyServiceStatusToClient(const ServiceID &sid, Availability oldStatus,
                                   Availability newStatus) override;
  bool onIncomingMessage(const CSMessagePtr &csMsg) override;

 protected:
  void onBytesCome(srz::Buffer &&buff) override;
  void notifyServiceStatusToClient(const Address &clAddr, const ServiceID &sid,
                                   Availability oldStatus,
                                   Availability newStatus);
  using RegistedClientAddresses = threading::Lockable<std::set<Address>>;
  RegistedClientAddresses registedClAddrs_;
  std::unique_ptr<BufferSenderIF> pSender_;
  std::unique_ptr<BufferReceiverIF> pReceiver_;
  std::thread listeningThread_;
};

std::shared_ptr<ServerIF> makeServer();

}  // namespace local
}  // namespace ipc
}  // namespace messaging
}  // namespace maf
