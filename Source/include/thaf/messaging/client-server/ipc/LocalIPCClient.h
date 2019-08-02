#pragma once

#include "IPCClientBase.h"
#include "thaf/patterns/Patterns.h"
#include "IPCMessageTrait.h"
#include "thaf/messaging/client-server/QueueingServiceProxy.h"

namespace thaf {
namespace messaging {
namespace ipc {

class LocalIPCClient : public IPCClientBase, public pattern::SingletonObject<LocalIPCClient>
{
public:
    LocalIPCClient(Invisible){}
    void init(const Address& addr, long long serverStatusCheckPeriodMS = 1000) {IPCClientBase::init(IPCType::Local, addr, serverStatusCheckPeriodMS); }
};

} // ipc
} // messaging
} // thaf
