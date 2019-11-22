#pragma once

#include "IPCClientBase.h"
#include <maf/patterns/Patterns.h>
#include <maf/messaging/client-server/SerializableMessageTrait.h>
#include <maf/messaging/client-server/QueueingServiceProxy.h>

namespace maf {
namespace messaging {
namespace ipc {

class LocalIPCClient : public IPCClientBase
{
public:
    LocalIPCClient() : IPCClientBase{ IPCType::Local }{}
};

} // ipc
} // messaging
} // maf
