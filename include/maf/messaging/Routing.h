#pragma once

#include <maf/export/MafExport_global.h>

#include "ComponentDef.h"

namespace maf {
namespace messaging {
namespace routing {

using Receiver = Component;
using ReceiverInstance = ComponentInstance;
using ReceiverRef = ComponentRef;
using ReceiverID = ComponentID;
using Message = Message;

struct receiver_status_update {
  enum availability { available, unavailable };
  ReceiverRef receiver;
  availability status = available;
  bool is_available() const { return status == available; }
  bool is_unavailable() const { return status == unavailable; }
};

MAF_EXPORT bool routeMessage(const ReceiverID& receiverID, Message msg);
MAF_EXPORT bool routeExecution(const ReceiverID& receiverID, Execution exc);

MAF_EXPORT bool routeMessageAndWait(const ReceiverID& receiverID, Message msg);
MAF_EXPORT bool routeAndWaitExecution(const ReceiverID& receiverID,
                                      Execution exc);
MAF_EXPORT bool broadcast(Message msg);
MAF_EXPORT ReceiverInstance findReceiver(const ReceiverID& id);

template <class Msg, typename... Args>
bool routeMessage(const ReceiverID& receiverID, Args&&... args) {
  using namespace std;
  return routeMessage(receiverID, makeMessage<Msg>(forward<Args>(args)...));
}
template <class Msg, typename... Args>
bool routeMessageAndWait(const ReceiverID& receiverID, Args&&... args) {
  using namespace std;
  return routeMessageAndWait(receiverID,
                             makeMessage<Msg>(forward<Args>(args)...));
}
template <class Msg, typename... Args>
bool broadcast(Args&&... args) {
  using namespace std;
  return broadcast(makeMessage<Msg>(forward<Args>(args)...));
}

}  // namespace routing
}  // namespace messaging
}  // namespace maf
