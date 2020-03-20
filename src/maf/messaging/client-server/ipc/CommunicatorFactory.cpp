#include "CommunicatorFactory.h"
#include "LocalIPCReceiver.h"
#include "LocalIPCSender.h"

namespace maf {
namespace messaging {
namespace ipc {

IPCSender *CommunicatorFactory::createSender(IPCType type) {
  switch (type) {
  case IPCType::Local:
    return new LocalIPCSender;
  default:
    return nullptr;
  }
}

IPCReceiver *CommunicatorFactory::createReceiver(IPCType type) {
  switch (type) {
  case IPCType::Local:
    return new LocalIPCReceiver;
  default:
    return nullptr;
  }
}

} // namespace ipc
} // namespace messaging
} // namespace maf
