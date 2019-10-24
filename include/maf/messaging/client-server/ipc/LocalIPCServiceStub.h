#pragma once

#include <maf/messaging/client-server/SSQServiceStub.h>
#include "LocalIPCServer.h"
#include "IPCMessageTrait.h"

namespace maf {
namespace messaging {
namespace ipc {


using LocalIPCClientRequestMsg = ClientRequestMessage<IPCMessageTrait>;
using LocalIPCServiceStub = SSQServiceStub<IPCMessageTrait, LocalIPCServer>;
using LocalIPCRequestKeeper = RequestKeeper<IPCMessageTrait>;

} // ipc
} // messaging
} // maf
