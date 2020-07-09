#include "Client.h"
#include "Server.h"

namespace maf {
namespace messaging {
namespace itc {

std::shared_ptr<Client> Client::instance() {
  static auto instance_ = std::make_shared<Client>();
  return instance_;
}

ActionCallStatus Client::sendMessageToServer(const CSMessagePtr &msg) {
  msg->setSourceAddress(
      Address{"", 0}); // TODO: later must be validated by validator
  if (Server::instance()->onIncomingMessage(msg)) {
    return ActionCallStatus::Success;
  } else {
    return ActionCallStatus::ReceiverUnavailable;
  }
}

} // namespace itc
} // namespace messaging
} // namespace maf
