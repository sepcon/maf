#pragma once

#include <maf/export/MafExport_global.h>

#include "ComponentDef.h"

namespace maf {
namespace messaging {

using Receiver = Component;
using ReceiverInstance = ComponentInstance;
using ReceiverRef = ComponentRef;
using ReceiverID = ComponentID;
using Message = ComponentMessage;

struct ReceiverStatusMsg {
    enum Status { Available, Unavailable };
    ReceiverRef receiver;
    Status status = Available;
    bool isAvailable() const { return status == Available; }
    bool isUnavailable() const { return status == Unavailable; }
};

MAF_EXPORT bool routeMessage(Message&& msg, const ReceiverID& receiverID);
MAF_EXPORT bool broadcast(Message&& msg);
MAF_EXPORT ReceiverInstance findReceiver(const ReceiverID& id);

}  // namespace messaging
}  // namespace maf
