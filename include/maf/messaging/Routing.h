#pragma once

#include <maf/export/MafExport_global.h>

#include "Processor.h"

namespace maf {
namespace messaging {
namespace routing {

struct ProcessorStatusUpdateMsg {
  enum class Status : char { Reachable, UnReachable };
  ProcessorRef compref;
  Status status = Status::Reachable;
  ProcessorInstance messageprocessor() const { return compref.lock(); }
  bool ready() const { return status == Status::Reachable; }
};

MAF_EXPORT bool post(const ProcessorID& messageprocessorID, Message msg);
MAF_EXPORT bool postToAll(Message msg);
MAF_EXPORT Processor::CompleteSignal send(const ProcessorID& messageprocessorID,
                                          Message msg);
MAF_EXPORT Processor::CompleteSignal sendToAll(Message msg);
MAF_EXPORT ProcessorInstance findProcessor(const ProcessorID& id);

template <class Msg, typename... Args>
bool post(const ProcessorID& messageprocessorID, Args&&... args) {
  using namespace std;
  return post(messageprocessorID, makeMessage<Msg>(forward<Args>(args)...));
}

template <class Msg, typename... Args>
bool postToAll(Args&&... args) {
  using namespace std;
  return postToAll(makeMessage<Msg>(forward<Args>(args)...));
}

template <class Msg, typename... Args>
Processor::CompleteSignal send(const ProcessorID& messageprocessorID,
                               Args&&... args) {
  using namespace std;
  return send(messageprocessorID, makeMessage<Msg>(forward<Args>(args)...));
}

template <class Msg, typename... Args>
Processor::CompleteSignal sendToAll(Args&&... args) {
  using namespace std;
  return sendToAll(makeMessage<Msg>(forward<Args>(args)...));
}

}  // namespace routing
}  // namespace messaging
}  // namespace maf
