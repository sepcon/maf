#pragma once

#include <maf/messaging/client-server/SCQServiceProxy.h>
#include "IPCMessageTrait.h"
#include "LocalIPCClient.h"
namespace maf {
namespace messaging {
namespace ipc {

using LocalIPCServiceProxy = SCQServiceProxy<IPCMessageTrait, LocalIPCClient>;

} // ipc
} // messaging
} // maf
