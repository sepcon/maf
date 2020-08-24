#pragma once

#include <maf/messaging/Component.h>
#include <maf/messaging/Routing.h>
#include <maf/patterns/Patterns.h>
#include <maf/threading/Lockable.h>

#include <mutex>
#include <set>

namespace maf {
namespace messaging {
namespace details {
using namespace routing;

struct ComponentCompare {
  typedef int is_transparent;
  bool operator()(const ComponentInstance &r1,
                  const ComponentInstance &r2) const {
    return r1 != r2 && r1->id() < r2->id();
  }

  bool operator()(const ComponentInstance &r, const ComponentID &id) const {
    return r->id() < id;
  }

  bool operator()(const ComponentID &id, const ComponentInstance &r) const {
    return id < r->id();
  }
};

using Components = std::set<ComponentInstance, ComponentCompare>;

class Router : public pattern::SingletonObject<Router> {
 public:
  Router(Invisible) noexcept {}
  bool postMsg(const ComponentID &componentID, Message &&msg);
  Component::MessageHandledSignal sendMsg(const ComponentID &componentID,
                                          Message msg);
  bool postMsg(const Message &msg);
  Component::MessageHandledSignal sendMsg(const Message &msg);

  ComponentInstance findComponent(const ComponentID &id) const;
  bool addComponent(ComponentInstance comp);
  bool removeComponent(const ComponentInstance &comp);

 private:
  using AtomicComponents = threading::Lockable<Components, std::mutex>;

  AtomicComponents components_;
};

}  // namespace details

using details::Router;

}  // namespace messaging
}  // namespace maf
