#include "IAMessageRouter.h"
#include <maf/messaging/client-server/ServiceProviderIF.h>
#include <maf/messaging/client-server/ServiceRequesterIF.h>

namespace maf {
namespace messaging {

std::shared_ptr<IAMessageRouter> IAMessageRouter::instance() {
  static std::shared_ptr<IAMessageRouter> __instance =
      std::make_shared<IAMessageRouter>();
  return __instance;
}

bool IAMessageRouter::deinit() {
  return ClientBase::deinit() && ClientBase::deinit();
}

ActionCallStatus IAMessageRouter::sendMessageToClient(const CSMessagePtr &msg,
                                                      const Address & /*addr*/
) {
  if (ClientBase::onIncomingMessage(msg)) {
    return ActionCallStatus::Success;
  } else {
    return ActionCallStatus::ReceiverUnavailable;
  }
}

ActionCallStatus IAMessageRouter::sendMessageToServer(const CSMessagePtr &msg) {
  msg->setSourceAddress(
      Address{"", 0}); // TODO: later must be validated by validator
  if (ServerBase::onIncomingMessage(msg)) {
    return ActionCallStatus::Success;
  } else {
    return ActionCallStatus::ReceiverUnavailable;
  }
}

void IAMessageRouter::notifyServiceStatusToClient(const ServiceID &sid,
                                                  Availability oldStatus,
                                                  Availability newStatus) {
  ClientBase::onServiceStatusChanged(sid, oldStatus, newStatus);
}

} // namespace messaging
} // namespace maf
