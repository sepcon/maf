#include <maf/messaging/MessageRouting.h>

#include "MessageRouter.h"

namespace maf {
namespace messaging {

bool routeMessage(Message &&msg, const ReceiverID &receiverID) {
  return MessageRouter::instance().route(std::move(msg), receiverID);
}

bool broadcast(Message &&msg) {
  return MessageRouter::instance().broadcast(std::move(msg));
}
ReceiverInstance findReceiver(const ReceiverID &id) {
  return MessageRouter::instance().findReceiver(id);
}

}  // namespace messaging
}  // namespace maf
