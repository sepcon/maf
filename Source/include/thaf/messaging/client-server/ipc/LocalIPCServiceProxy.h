#pragma once

#include "thaf/messaging/client-server/QueueingServiceProxy.h"
#include "IPCMessageTrait.h"

namespace thaf {
namespace messaging {
namespace ipc {

using LocalIPCServiceProxy = QueueingServiceProxy<IPCMessageTrait>;

} // ipc
} // messaging
} // thaf
