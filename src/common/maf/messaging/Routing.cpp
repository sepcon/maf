#include <maf/messaging/Routing.h>

#include "Router.h"

namespace maf {
namespace messaging {
namespace routing {

bool post(const ComponentID &componentID, Message msg) {
  return Router::instance().post(componentID, std::move(msg));
}

bool postToAll(Message msg) {
  return Router::instance().postToAll(std::move(msg));
}

ComponentInstance findComponent(const ComponentID &id) {
  return Router::instance().findComponent(id);
}

Component::CompleteSignal send(const ComponentID &componentID,
                                     Message msg) {
  return Router::instance().send(componentID, std::move(msg));
}

Component::CompleteSignal sendToAll(Message msg) {
  return Router::instance().sendToAll(std::move(msg));
}

}  // namespace routing
}  // namespace messaging
}  // namespace maf
