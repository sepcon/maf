#pragma once

#include "IPCServerBase.h"
#include "thaf/patterns/Patterns.h"
#include "IPCMessageTrait.h"

namespace thaf {
namespace messaging {
namespace ipc {

class LocalIPCServer : public IPCServerBase, public pattern::SingletonObject<LocalIPCServer>
{
public:
    LocalIPCServer(Invisible) : IPCServerBase() {}
    void init(const Address& addr) { IPCServerBase::init(IPCType::Local, addr); }
};
} // ipc
} // messaging
} // thaf