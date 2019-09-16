#pragma once

#include "IPCServerBase.h"
#include <maf/patterns/Patterns.h>
#include "IPCMessageTrait.h"

namespace maf {
namespace messaging {
namespace ipc {

class LocalIPCServer : public IPCServerBase, public pattern::SingletonObject<LocalIPCServer>
{
public:
    LocalIPCServer(Invisible){}
    void init(const Address& addr) { IPCServerBase::init(IPCType::Local, addr); }
};


} // ipc
} // messaging
} // maf
