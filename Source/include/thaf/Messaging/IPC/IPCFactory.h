#ifndef IPCFACTORY_H
#define IPCFACTORY_H

#include "IPCSender.h"
#include "IPCReceiver.h"
#include <memory>

namespace thaf {
namespace messaging {
namespace ipc {

enum class IPCType : unsigned char
{
    LocalScope = 0,
    NotSpecified
};

class IPCFactory
{
public:
    static std::shared_ptr<IPCSender> createSender(IPCType type);
    static std::shared_ptr<IPCReceiver> createReceiver(IPCType type);
};

} // ipc
} // messaging
} // thaf
#endif // IPCFACTORY_H
