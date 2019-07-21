#pragma once

#include "thaf/messaging/client-server/QueueingServiceProxy.h"
#include "LocalIPCClient.h"
#include "IPCMessageTrait.h"

namespace thaf {
namespace messaging {
namespace ipc {

using LocalIPCServiceProxy = QueueingServiceProxy<IPCMessageTrait, LocalIPCClient>;

} // ipc
} // messaging
} // thaf
