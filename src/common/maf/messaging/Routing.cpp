#include <maf/messaging/Routing.h>

#include "Router.h"

namespace maf {
namespace messaging {
namespace routing {

bool routeMessage(Message &&msg, const ReceiverID &receiverID) {
  return Router::instance().routeMessage(std::move(msg), receiverID);
}

bool routeExecution(Execution exc, const ReceiverID &receiverID) {
  return Router::instance().routeExecution(std::move(exc), receiverID);
}

bool broadcast(Message &&msg) {
  return Router::instance().broadcast(std::move(msg));
}
ReceiverInstance findReceiver(const ReceiverID &id) {
  return Router::instance().findReceiver(id);
}

}  // namespace routing
}  // namespace messaging
}  // namespace maf
