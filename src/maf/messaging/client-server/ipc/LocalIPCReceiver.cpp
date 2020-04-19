#include "LocalIPCReceiver.h"
#include <internal/LocalIPCReceiverImpl.h>

namespace maf {
namespace messaging {
namespace ipc {

LocalIPCReceiver::LocalIPCReceiver() {
  _impl = std::make_unique<LocalIPCReceiverImpl>();
}

LocalIPCReceiver::~LocalIPCReceiver() {}

bool LocalIPCReceiver::initConnection(const Address &address,
                                      bool isClientMode) {
  return _impl->initConnection(address, isClientMode);
}

bool LocalIPCReceiver::startListening() { return _impl->startListening(); }

bool LocalIPCReceiver::stopListening() { return _impl->stopListening(); }

bool LocalIPCReceiver::listening() const { return _impl->listening(); }

const Address &LocalIPCReceiver::address() const { return _impl->address(); }

void LocalIPCReceiver::registerObserver(BytesComeObserver *observer) {
  _impl->registerObserver(
      [observer](auto &&bytes) { observer->onBytesCome(std::move(bytes)); });
}

} // namespace ipc
} // namespace messaging
} // namespace maf
