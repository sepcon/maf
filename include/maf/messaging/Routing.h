#pragma once

#include <maf/export/MafExport_global.h>

#include "Component.h"
#include "ComponentRequest.h"

namespace maf {
namespace messaging {
namespace routing {

struct ComponentStatusUpdateMsg {
  enum class Status : char { Reachable, UnReachable };
  ComponentRef compref;
  Status status = Status::Reachable;
  ComponentInstance component() const { return compref.lock(); }
  bool ready() const { return status == Status::Reachable; }
};

MAF_EXPORT bool post(const ComponentID& componentID, Message msg);
MAF_EXPORT bool postToAll(Message msg);
MAF_EXPORT Component::MessageHandledSignal send(const ComponentID& componentID,
                                                Message msg);
MAF_EXPORT Component::MessageHandledSignal sendToAll(Message msg);
MAF_EXPORT ComponentInstance findComponent(const ComponentID& id);

template <class Msg, typename... Args>
bool post(const ComponentID& componentID, Args&&... args) {
  using namespace std;
  return post(componentID, makeMessage<Msg>(forward<Args>(args)...));
}

template <class Msg, typename... Args>
bool postToAll(Args&&... args) {
  using namespace std;
  return postToAll(makeMessage<Msg>(forward<Args>(args)...));
}

template <class Msg, typename... Args>
Component::MessageHandledSignal send(const ComponentID& componentID,
                                     Args&&... args) {
  using namespace std;
  return send(componentID, makeMessage<Msg>(forward<Args>(args)...));
}

template <class Msg, typename... Args>
Component::MessageHandledSignal sendToAll(Args&&... args) {
  using namespace std;
  return sendToAll(makeMessage<Msg>(forward<Args>(args)...));
}

template <class Output, class Input, RequestType type>
ComponentRequest<Output, Input, type> makeRequest(
    const ComponentID& componentID) {
  return ComponentRequest<Output, Input, type>{findComponent(componentID)};
}

template <class Output, class Input>
auto makeRequestAsync(const ComponentID& componentID) {
  return makeRequest<Output, Input, RequestType::Async>(componentID);
}

template <class Output, class Input>
auto makeRequestSync(const ComponentID& componentID) {
  return makeRequest<Output, Input, RequestType::Sync>(componentID);
}

}  // namespace routing
}  // namespace messaging
}  // namespace maf
