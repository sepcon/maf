#pragma once

#include "IPCServerBase.h"
#include <maf/patterns/Patterns.h>
#include <maf/messaging/client-server/SerializableMessageTrait.h>

namespace maf {
namespace messaging {
namespace ipc {

class LocalIPCServer : public IPCServerBase, pattern::Unasignable
{
public:
    LocalIPCServer() : IPCServerBase ( IPCType::Local ) {}
};


} // ipc
} // messaging
} // maf
