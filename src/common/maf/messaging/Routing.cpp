#include <maf/messaging/Routing.h>

#include "MessageRouter.h"

namespace maf {
namespace messaging {
namespace routing {

bool routeMessage(Message &&msg, const ReceiverID &receiverID) {
  return MessageRouter::instance().route(std::move(msg), receiverID);
}

bool broadcast(Message &&msg) {
  return MessageRouter::instance().broadcast(std::move(msg));
}
ReceiverInstance findReceiver(const ReceiverID &id) {
  return MessageRouter::instance().findReceiver(id);
}

}  // namespace routing
}  // namespace messaging
}  // namespace maf
