#pragma once

#include <maf/messaging/Component.h>
#include <maf/messaging/Routing.h>
#include <maf/patterns/Patterns.h>
#include <maf/threading/Lockable.h>

#include <mutex>
#include <set>

namespace maf {
namespace messaging {
namespace impl {
using namespace routing;

struct ReceiverCompare {
  typedef int is_transparent;
  bool operator()(const ReceiverInstance &r1,
                  const ReceiverInstance &r2) const {
    return r1 != r2 && r1->id() < r2->id();
  }

  bool operator()(const ReceiverInstance &r, const ReceiverID &id) const {
    return r->id() < id;
  }

  bool operator()(const ReceiverID &id, const ReceiverInstance &r) const {
    return id < r->id();
  }
};

class Router : public pattern::SingletonObject<Router> {
 public:
  using Receivers = std::set<ReceiverInstance, ReceiverCompare>;

  Router(Invisible) noexcept {}
  bool routeMessage(const ReceiverID &receiverID, Message &&msg);
  bool routeExecution(const ReceiverID &receiverID, Execution exc);
  bool routeMessageAndWait(const ReceiverID &receiverID, Message &&msg);
  bool routeAndWaitExecution(const ReceiverID &receiverID, Execution exc);
  bool broadcast(const Message &msg);
  ReceiverInstance findReceiver(const ReceiverID &id) const;
  bool addReceiver(ReceiverInstance receiver);
  bool removeReceiver(const ReceiverInstance &receiver);

 private:
  using AtomicReceivers = threading::Lockable<Receivers, std::mutex>;

  AtomicReceivers receivers_;
};

}  // namespace impl

using impl::Router;

}  // namespace messaging
}  // namespace maf
