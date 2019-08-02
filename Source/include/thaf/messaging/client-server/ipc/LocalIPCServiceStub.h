#pragma once

#include <thaf/messaging/client-server/SSQServiceStub.h>
#include "LocalIPCServer.h"
#include "IPCMessageTrait.h"

namespace thaf {
namespace messaging {
namespace ipc {


using IPCClientRequestMsg = ClientRequestMessage<IPCMessageTrait>;
using LocalIPCServiceStub = SSQServiceStub<IPCMessageTrait, LocalIPCServer>;

} // ipc
} // messaging
} // thaf
