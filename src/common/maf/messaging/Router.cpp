#include "Router.h"

namespace maf {
namespace messaging {
namespace impl {

static bool askThenPost(const ReceiverInstance &r, Message msg);
static void notifyAllAboutNewReceiver(const Receivers &joinedReceivers,
                                      const ReceiverInstance &newReceiver);
static void informNewReceiverAboutJoinedOnes(
    const ReceiverInstance &newReceiver, const Receivers &joinedReceivers);

bool Router::routeMessage(const ReceiverID &receiverID, Message &&msg) {
  if (auto receiver = findReceiver(receiverID)) {
    return receiver->post(std::move(msg));
  }
  return false;
}

bool Router::routeExecution(const ReceiverID &receiverID, Execution exc) {
  if (auto receiver = findReceiver(receiverID)) {
    return receiver->execute(std::move(exc));
  }
  return false;
}

bool Router::routeMessageAndWait(const ReceiverID &receiverID, Message &&msg) {
  if (auto receiver = findReceiver(receiverID)) {
    return receiver->postAndWait(std::move(msg));
  }
  return false;
}

bool Router::routeAndWaitExecution(const ReceiverID &receiverID,
                                   Execution exc) {
  if (auto receiver = findReceiver(receiverID)) {
    return receiver->executeAndWait(std::move(exc));
  }
  return false;
}

bool Router::broadcast(const Message &msg) {
  bool delivered = false;
  auto atReceivers = receivers_.atomic();
  for (const auto &receiver : *atReceivers) {
    delivered |= askThenPost(receiver, std::move(msg));
  }
  return delivered;
}

ReceiverInstance Router::findReceiver(const ReceiverID &id) const {
  auto atReceivers = receivers_.atomic();
  if (auto itReceiver = atReceivers->find(id);
      itReceiver != atReceivers->end()) {
    return *itReceiver;
  }
  return {};
}

bool Router::addReceiver(ReceiverInstance newReceiver) {
  if (newReceiver) {
    auto joinedReceivers = receivers_.atomic();
    if (joinedReceivers->count(newReceiver) == 0) {
      informNewReceiverAboutJoinedOnes(newReceiver, *joinedReceivers);
      notifyAllAboutNewReceiver(*joinedReceivers, newReceiver);
      joinedReceivers->insert(move(newReceiver));
    }
  }
  return false;
}

bool Router::removeReceiver(const ReceiverInstance &receiver) {
  if (receivers_.atomic()->erase(receiver) != 0) {
    broadcast(receiver_status_update{
        receiver, receiver_status_update::unavailable});
    return true;
  }
  return false;
}

static bool askThenPost(const ReceiverInstance &r, Message msg) {
  if (r->hasHandler(msg.type())) {
    return r->post(std::move(msg));
  }
  return false;
}

static void notifyAllAboutNewReceiver(const Receivers &joinedReceivers,
                                      const ReceiverInstance &newReceiver) {
  auto msg = receiver_status_update{
      newReceiver, receiver_status_update::available};

  for (const auto &joinedReceiver : joinedReceivers) {
    askThenPost(joinedReceiver, msg);
  }
}

static void informNewReceiverAboutJoinedOnes(
    const ReceiverInstance &newReceiver, const Receivers &joinedReceivers) {
    if (newReceiver->hasHandler(msgid<receiver_status_update>())) {
    for (const auto &joinedOne : joinedReceivers) {
      newReceiver->post<receiver_status_update>(
          joinedOne, receiver_status_update::available);
    }
  }
}

}  // namespace impl
}  // namespace messaging
}  // namespace maf
