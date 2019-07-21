#pragma once

#include "thaf/messaging/client-server/QueueingServiceStub.h"
#include "LocalIPCServer.h"
#include "IPCMessageTrait.h"

namespace thaf {
namespace messaging {
namespace ipc {


using IPCClientRequestMsg = ClientRequestMessage<IPCMessageTrait, LocalIPCServer>;
using LocalIPCServiceStub = QueueingServiceStub<IPCMessageTrait, LocalIPCServer>;

} // ipc
} // messaging
} // thaf
