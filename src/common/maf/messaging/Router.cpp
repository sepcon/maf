#include "Router.h"

#include <vector>

namespace maf {
namespace messaging {
namespace details {
using namespace std;

static bool askThenPost(const ProcessorInstance &r, Message msg);
static Processor::CompleteSignal askThenSend(const ProcessorInstance &r,
                                             Message msg);
static void notifyAllAboutNewProcessor(const Processors &joinedProcessors,
                                       const ProcessorInstance &newProcessor);
static void informNewProcessorAboutJoinedOnes(
    const ProcessorInstance &newProcessor, const Processors &joinedProcessors);

bool Router::post(const ProcessorID &messageprocessorID, Message &&msg) {
  if (auto comp = findProcessor(messageprocessorID)) {
    return comp->post(std::move(msg));
  }
  return false;
}

Processor::CompleteSignal Router::send(const ProcessorID &messageprocessorID,
                                       Message msg) {
  if (auto comp = findProcessor(messageprocessorID)) {
    return comp->waitablePost(std::move(msg));
  }
  return {};
}

bool Router::postToAll(const Message &msg) {
  bool delivered = false;
  auto atProcessors = messageprocessors_.atomic();
  for (const auto &comp : *atProcessors) {
    delivered |= askThenPost(comp, std::move(msg));
  }
  return delivered;
}

Processor::CompleteSignal Router::sendToAll(const Message &msg) {
  auto msgMessageHandledSignals = vector<Processor::CompleteSignal>{};
  auto atProcessors = messageprocessors_.atomic();
  for (const auto &comp : *atProcessors) {
    if (auto sig = askThenSend(comp, std::move(msg)); sig.valid()) {
      msgMessageHandledSignals.emplace_back(move(sig));
    }
  }

  if (!msgMessageHandledSignals.empty()) {
    return Processor::CompleteSignal{async(
        launch::deferred, [sigs{move(msgMessageHandledSignals)}]() mutable {
          for (auto &sig : sigs) {
            sig.get();
          }
        })};
  } else {
    return {};
  }
}

ProcessorInstance Router::findProcessor(const ProcessorID &id) const {
  auto atProcessors = messageprocessors_.atomic();
  if (auto itProcessor = atProcessors->find(id);
      itProcessor != atProcessors->end()) {
    return *itProcessor;
  }
  return {};
}

bool Router::addProcessor(ProcessorInstance comp) {
  if (comp) {
    auto joinedProcessors = messageprocessors_.atomic();
    if (joinedProcessors->count(comp) == 0) {
      informNewProcessorAboutJoinedOnes(comp, *joinedProcessors);
      notifyAllAboutNewProcessor(*joinedProcessors, comp);
      joinedProcessors->insert(move(comp));
    }
  }
  return false;
}

bool Router::removeProcessor(const ProcessorInstance &comp) {
  if (messageprocessors_.atomic()->erase(comp) != 0) {
    postToAll(ProcessorStatusUpdateMsg{
        comp, ProcessorStatusUpdateMsg::Status::UnReachable});
    return true;
  }
  return false;
}

static bool askThenPost(const ProcessorInstance &r, Message msg) {
  if (r->connected(msg.type())) {
    return r->post(std::move(msg));
  }
  return false;
}

static Processor::CompleteSignal askThenSend(const ProcessorInstance &r,
                                             Message msg) {
  if (r->connected(msg.type())) {
    return r->waitablePost(std::move(msg));
  }
  return {};
}

static void notifyAllAboutNewProcessor(const Processors &joinedProcessors,
                                       const ProcessorInstance &newProcessor) {
  auto msg = ProcessorStatusUpdateMsg{
      newProcessor, ProcessorStatusUpdateMsg::Status::Reachable};

  for (const auto &joinedProcessor : joinedProcessors) {
    askThenPost(joinedProcessor, msg);
  }
}

static void informNewProcessorAboutJoinedOnes(
    const ProcessorInstance &newProcessor, const Processors &joinedProcessors) {
  if (newProcessor->connected(msgid<ProcessorStatusUpdateMsg>())) {
    for (const auto &joinedOne : joinedProcessors) {
      newProcessor->post<ProcessorStatusUpdateMsg>(
          joinedOne, ProcessorStatusUpdateMsg::Status::Reachable);
    }
  }
}

}  // namespace details
}  // namespace messaging
}  // namespace maf
