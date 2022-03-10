#include "Server.h"

#include "../ServerBase.h"
#include "Client.h"

namespace maf {
namespace messaging {
namespace itc {

auto client() { return makeClient(); }

class Server : public ServerBase, pattern::Unasignable {
  friend class Client;

 public:
  static std::shared_ptr<Server> instance();
  bool start() override { return true; }
  void stop() override {}
  void deinit() override {}
  ActionCallStatus sendMessageToClient(const CSMessagePtr &msg,
                                       const Address &addr = {}) override;
  void notifyServiceStatusToClient(const ServiceID &sid, Availability oldStatus,
                                   Availability newStatus) override;
};

std::shared_ptr<Server> Server::instance() {
  static auto instance_ = std::make_shared<Server>();
  return instance_;
}

ActionCallStatus Server::sendMessageToClient(const CSMessagePtr &msg,
                                             const Address & /*addr*/) {
  if (client()->onIncomingMessage(msg)) {
    return ActionCallStatus::Success;
  } else {
    return ActionCallStatus::ReceiverUnavailable;
  }
}

void Server::notifyServiceStatusToClient(const ServiceID &sid,
                                         Availability oldStatus,
                                         Availability newStatus) {
  client()->onServiceStatusChanged(sid, oldStatus, newStatus);
}

std::shared_ptr<ServerIF> makeServer() { return Server::instance(); }

}  // namespace itc
}  // namespace messaging
}  // namespace maf
