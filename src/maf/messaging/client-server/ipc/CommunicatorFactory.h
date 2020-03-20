#ifndef IPCFACTORY_H
#define IPCFACTORY_H

#include "IPCTypes.h"
#include <memory>

namespace maf {
namespace messaging {
namespace ipc {

class IPCSender;
class IPCReceiver;

class CommunicatorFactory {
public:
  static IPCSender *createSender(IPCType type);
  static IPCReceiver *createReceiver(IPCType type);
};

} // namespace ipc
} // namespace messaging
} // namespace maf
#endif // IPCFACTORY_H
