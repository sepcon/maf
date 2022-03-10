#include <maf/messaging/Routing.h>

#include "Router.h"

namespace maf {
namespace messaging {
namespace routing {

bool post(const ProcessorID &messageprocessorID, Message msg) {
  return Router::instance().post(messageprocessorID, std::move(msg));
}

bool postToAll(Message msg) {
  return Router::instance().postToAll(std::move(msg));
}

ProcessorInstance findProcessor(const ProcessorID &id) {
  return Router::instance().findProcessor(id);
}

Processor::CompleteSignal send(const ProcessorID &messageprocessorID,
                                     Message msg) {
  return Router::instance().send(messageprocessorID, std::move(msg));
}

Processor::CompleteSignal sendToAll(Message msg) {
  return Router::instance().sendToAll(std::move(msg));
}

}  // namespace routing
}  // namespace messaging
}  // namespace maf
