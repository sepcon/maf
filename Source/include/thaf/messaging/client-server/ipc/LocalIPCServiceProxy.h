#pragma once

#include <thaf/messaging/client-server/SCQServiceProxy.h>
#include "IPCMessageTrait.h"
#include "LocalIPCClient.h"
namespace thaf {
namespace messaging {
namespace ipc {

using LocalIPCServiceProxy = SCQServiceProxy<IPCMessageTrait, LocalIPCClient>;

} // ipc
} // messaging
} // thaf
