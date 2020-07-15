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

class MessageRouter : public pattern::SingletonObject<MessageRouter> {
public:
  using Receivers = std::set<ReceiverInstance, ReceiverCompare>;

  MessageRouter(Invisible) noexcept {}
  bool route(Message &&msg, const ReceiverID &receiverID) noexcept;
  bool broadcast(const Message &msg) noexcept;
  ReceiverInstance findReceiver(const ReceiverID &id) const noexcept;

  bool addReceiver(ReceiverInstance receiver) noexcept;
  bool removeReceiver(const ReceiverInstance &receiver) noexcept;

private:
  using AtomicReceivers = threading::Lockable<Receivers, std::mutex>;

  AtomicReceivers receivers_;
};

} // namespace impl

using impl::MessageRouter;

} // namespace messaging
} // namespace maf
