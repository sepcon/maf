#ifndef IPCFACTORY_H
#define IPCFACTORY_H

#include "IPCTypes.h"
#include "IPCSender.h"
#include "IPCReceiver.h"
#include <memory>

namespace maf {
namespace messaging {
namespace ipc {

class IPCFactory
{
public:
    static std::shared_ptr<IPCSender> createSender(IPCType type);
    static std::shared_ptr<IPCReceiver> createReceiver(IPCType type);
};

} // ipc
} // messaging
} // maf
#endif // IPCFACTORY_H
