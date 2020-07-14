#include "MessageRouter.h"

namespace maf {
namespace messaging {
namespace impl {

static inline void broadcastExcept(Message &&msg,
                                   const MessageRouter::Receivers &receivers,
                                   const ReceiverInstance &ignoredRcvr) {
  for (auto &r : receivers) {
    if (r != ignoredRcvr) {
      r->post(msg);
    }
  }
}

bool MessageRouter::route(Message &&msg,
                          const ReceiverID &receiverID) noexcept {
  if (auto receiver = findReceiver(receiverID)) {
    return receiver->post(std::move(msg));
  }
  return false;
}

bool MessageRouter::broadcast(const Message &msg) noexcept {
  bool delivered = false;
  auto atReceivers = receivers_.atomic();
  for (const auto &receiver : *atReceivers) {
    delivered |= receiver->post(msg);
  }
  return delivered;
}

ReceiverInstance MessageRouter::findReceiver(
    const ReceiverID &id) const noexcept {
  auto atReceivers = receivers_.atomic();
  if (auto itReceiver = atReceivers->find(id);
      itReceiver != atReceivers->end()) {
    return *itReceiver;
  }
  return {};
}

bool MessageRouter::addReceiver(ReceiverInstance receiver) noexcept {
  if (receiver) {
    auto atmReceivers = receivers_.atomic();
    if (atmReceivers->insert(receiver).second) {
      broadcastExcept(ReceiverStatusMsg{receiver, ReceiverStatusMsg::Available},
                      *atmReceivers, receiver);
      return true;
    }
  }
  return false;
}

bool MessageRouter::removeReceiver(const ReceiverInstance &receiver) noexcept {
  if (receivers_.atomic()->erase(receiver) != 0) {
    broadcast(ReceiverStatusMsg{receiver, ReceiverStatusMsg::Unavailable});
    return true;
  }
  return false;
}

}  // namespace impl
}  // namespace messaging
}  // namespace maf
