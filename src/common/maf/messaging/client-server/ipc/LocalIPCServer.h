#pragma once

#include "../ServerBase.h"
#include "IPCReceiverIF.h"
#include "IPCTypes.h"
#include <set>
#include <thread>

namespace maf {
namespace messaging {
namespace ipc {

class IPCSenderIF;
class IPCReceiverIF;
class LocalIPCServer : public ServerBase, public BytesComeObserver {
public:
  LocalIPCServer();
  ~LocalIPCServer() override;
  bool init(const Address &serverAddress) override;
  bool start() override;
  void stop() override;
  void deinit() override;

  ActionCallStatus sendMessageToClient(const CSMessagePtr &msg,
                                       const Address &addr) override;
  void notifyServiceStatusToClient(const ServiceID &sid, Availability oldStatus,
                                   Availability newStatus) override;
  bool onIncomingMessage(const CSMessagePtr &csMsg) override;

protected:
  void onBytesCome(srz::ByteArray &&bytes) override;
  void notifyServiceStatusToClient(const Address &clAddr, const ServiceID &sid,
                                   Availability oldStatus,
                                   Availability newStatus);
  using RegistedClientAddresses = threading::Lockable<std::set<Address>>;
  RegistedClientAddresses registedClAddrs_;
  std::unique_ptr<IPCSenderIF> pSender_;
  std::unique_ptr<IPCReceiverIF> pReceiver_;
  std::thread listeningThread_;
};
} // namespace ipc
} // namespace messaging
} // namespace maf
