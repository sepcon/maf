#include <maf/messaging/Routing.h>

#include "Router.h"

namespace maf {
namespace messaging {
namespace routing {

bool postMsg(const ComponentID &componentID, Message msg) {
  return Router::instance().postMsg(componentID, std::move(msg));
}

bool postMsg(Message msg) { return Router::instance().postMsg(std::move(msg)); }

ComponentInstance findComponent(const ComponentID &id) {
  return Router::instance().findComponent(id);
}

Component::MessageHandledSignal sendMsg(const ComponentID &componentID,
                                        Message msg) {
  return Router::instance().sendMsg(componentID, std::move(msg));
}

Component::MessageHandledSignal sendMsg(Message msg) {
  return Router::instance().sendMsg(std::move(msg));
}

}  // namespace routing
}  // namespace messaging
}  // namespace maf
