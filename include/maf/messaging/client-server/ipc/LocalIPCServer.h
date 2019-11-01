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
    LocalIPCServer(Invisible) : IPCServerBase ( IPCType::Local ) {}
};


} // ipc
} // messaging
} // maf
