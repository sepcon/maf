#include "thaf/Messaging/IPC/IPCFactory.h"

#include "thaf/Messaging/IPC/Prv/LocalIPCReceiver.h"
#include "thaf/Messaging/IPC/Prv/LocalIPCSender.h"

namespace thaf {
namespace messaging {
namespace ipc {

std::shared_ptr<IPCSender> IPCFactory::createSender(IPCType type)
{
    switch (type) {
    case IPCType::LocalScope:
        return std::make_shared<LocalIPCSender>();
    default:
        return nullptr;
    }
}

std::shared_ptr<IPCReceiver> IPCFactory::createReceiver(IPCType type)
{
    switch (type) {
    case IPCType::LocalScope:
        return std::make_shared<LocalIPCReceiver>();
    default:
        return nullptr;
    }
}


} // ipc
} // messaging
} // thaf
