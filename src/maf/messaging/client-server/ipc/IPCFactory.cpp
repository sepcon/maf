#include <maf/messaging/client-server/ipc/IPCFactory.h>
#include "LocalIPCReceiver.h"
#include "LocalIPCSender.h"

namespace maf {
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
} // maf
