#pragma once

#include "IPCClientBase.h"
#include "thaf/patterns/Patterns.h"
#include "IPCMessageTrait.h"

namespace thaf {
namespace messaging {
namespace ipc {

class LocalIPCClient : public IPCClientBase, public pattern::SingletonObject<LocalIPCClient>
{
public:
    LocalIPCClient(Invisible) : IPCClientBase(){}
    void init(const Address& addr) {IPCClientBase::init(IPCType::Local, addr); }
};

} // ipc
} // messaging
} // thaf
