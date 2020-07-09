#include "Server.h"
#include "Client.h"

namespace maf {
namespace messaging {
namespace itc {

std::shared_ptr<Server> Server::instance() {
  static auto instance_ = std::make_shared<Server>();
  return instance_;
}

ActionCallStatus Server::sendMessageToClient(const CSMessagePtr &msg,
                                             const Address &/*addr*/) {
  if (Client::instance()->onIncomingMessage(msg)) {
    return ActionCallStatus::Success;
  } else {
    return ActionCallStatus::ReceiverUnavailable;
  }
}

void Server::notifyServiceStatusToClient(const ServiceID &sid,
                                         Availability oldStatus,
                                         Availability newStatus) {

  Client::instance()->onServiceStatusChanged(sid, oldStatus, newStatus);
}

} // namespace itc
} // namespace messaging
} // namespace maf
