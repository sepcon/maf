#include <maf/messaging/Routing.h>

#include "Router.h"

namespace maf {
namespace messaging {
namespace routing {

bool routeMessage(const ReceiverID &receiverID, Message msg) {
  return Router::instance().routeMessage(receiverID, std::move(msg));
}

bool routeExecution(const ReceiverID &receiverID, Execution exc) {
  return Router::instance().routeExecution(receiverID, std::move(exc));
}

bool broadcast(Message msg) {
  return Router::instance().broadcast(std::move(msg));
}
ReceiverInstance findReceiver(const ReceiverID &id) {
  return Router::instance().findReceiver(id);
}

bool routeAndWaitExecution(const ReceiverID &receiverID, Execution exc) {
  return Router::instance().routeAndWaitExecution(receiverID, std::move(exc));
}

bool routeMessageAndWait(const ReceiverID &receiverID, Message msg) {
  return Router::instance().routeMessageAndWait(receiverID, std::move(msg));
}

}  // namespace routing
}  // namespace messaging
}  // namespace maf
