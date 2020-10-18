#include "Router.h"

namespace maf {
namespace messaging {
namespace details {

static bool askThenPost(const ComponentInstance &r, Message msg);
static Component::CompleteSignal askThenSend(const ComponentInstance &r,
                                             Message msg);
static void notifyAllAboutNewComponent(const Components &joinedComponents,
                                       const ComponentInstance &newComponent);
static void informNewComponentAboutJoinedOnes(
    const ComponentInstance &newComponent, const Components &joinedComponents);

bool Router::post(const ComponentID &componentID, Message &&msg) {
  if (auto comp = findComponent(componentID)) {
    return comp->post(std::move(msg));
  }
  return false;
}

Component::CompleteSignal Router::send(const ComponentID &componentID,
                                       Message msg) {
  if (auto comp = findComponent(componentID)) {
    return comp->send(std::move(msg));
  }
  return {};
}

bool Router::postToAll(const Message &msg) {
  bool delivered = false;
  auto atComponents = components_.atomic();
  for (const auto &comp : *atComponents) {
    delivered |= askThenPost(comp, std::move(msg));
  }
  return delivered;
}

Component::CompleteSignal Router::sendToAll(const Message &msg) {
  auto msgMessageHandledSignals = vector<Component::CompleteSignal>{};
  auto atComponents = components_.atomic();
  for (const auto &comp : *atComponents) {
    if (auto sig = askThenSend(comp, std::move(msg)); sig.valid()) {
      msgMessageHandledSignals.emplace_back(move(sig));
    }
  }

  if (!msgMessageHandledSignals.empty()) {
    return Component::CompleteSignal{async(
        launch::deferred, [sigs{move(msgMessageHandledSignals)}]() mutable {
          for (auto &sig : sigs) {
            sig.get();
          }
        })};
  } else {
    return {};
  }
}

ComponentInstance Router::findComponent(const ComponentID &id) const {
  auto atComponents = components_.atomic();
  if (auto itComponent = atComponents->find(id);
      itComponent != atComponents->end()) {
    return *itComponent;
  }
  return {};
}

bool Router::addComponent(ComponentInstance comp) {
  if (comp) {
    auto joinedComponents = components_.atomic();
    if (joinedComponents->count(comp) == 0) {
      informNewComponentAboutJoinedOnes(comp, *joinedComponents);
      notifyAllAboutNewComponent(*joinedComponents, comp);
      joinedComponents->insert(move(comp));
    }
  }
  return false;
}

bool Router::removeComponent(const ComponentInstance &comp) {
  if (components_.atomic()->erase(comp) != 0) {
    postToAll(ComponentStatusUpdateMsg{
        comp, ComponentStatusUpdateMsg::Status::UnReachable});
    return true;
  }
  return false;
}

static bool askThenPost(const ComponentInstance &r, Message msg) {
  if (r->connected(msg.type())) {
    return r->post(std::move(msg));
  }
  return false;
}

static Component::CompleteSignal askThenSend(const ComponentInstance &r,
                                             Message msg) {
  if (r->connected(msg.type())) {
    return r->send(std::move(msg));
  }
  return {};
}

static void notifyAllAboutNewComponent(const Components &joinedComponents,
                                       const ComponentInstance &newComponent) {
  auto msg = ComponentStatusUpdateMsg{
      newComponent, ComponentStatusUpdateMsg::Status::Reachable};

  for (const auto &joinedComponent : joinedComponents) {
    askThenPost(joinedComponent, msg);
  }
}

static void informNewComponentAboutJoinedOnes(
    const ComponentInstance &newComponent, const Components &joinedComponents) {
  if (newComponent->connected(msgid<ComponentStatusUpdateMsg>())) {
    for (const auto &joinedOne : joinedComponents) {
      newComponent->post<ComponentStatusUpdateMsg>(
          joinedOne, ComponentStatusUpdateMsg::Status::Reachable);
    }
  }
}

}  // namespace details
}  // namespace messaging
}  // namespace maf
