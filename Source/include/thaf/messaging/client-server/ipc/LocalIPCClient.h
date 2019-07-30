#pragma once

#include "IPCClientBase.h"
#include "thaf/patterns/Patterns.h"
#include "IPCMessageTrait.h"
#include "thaf/messaging/client-server/QueueingServiceProxy.h"

namespace thaf {
namespace messaging {
namespace ipc {

class LocalIPCClient : public IPCClientBase
{
public:
    void init(const Address& addr) {IPCClientBase::init(IPCType::Local, addr); }
};

} // ipc
} // messaging
} // thaf
