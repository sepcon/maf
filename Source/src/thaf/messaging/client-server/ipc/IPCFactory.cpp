#include "thaf/messaging/client-server/ipc/IPCFactory.h"
#include "thaf/messaging/client-server/ipc/internal/LocalIPCReceiver.h"
#include "thaf/messaging/client-server/ipc/internal/LocalIPCSender.h"

namespace thaf {
namespace messaging {
namespace ipc {

std::shared_ptr<IPCSender> IPCFactory::createSender(IPCType type)
{
    switch (type) {
    case IPCType::Local:
        return std::make_shared<LocalIPCSender>();
    default:
        return nullptr;
    }
}

std::shared_ptr<IPCReceiver> IPCFactory::createReceiver(IPCType type)
{
    switch (type) {
    case IPCType::Local:
        return std::make_shared<LocalIPCReceiver>();
    default:
        return nullptr;
    }
}


} // ipc
} // messaging
} // thaf
