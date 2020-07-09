#include "LocalIPCReceiver.h"
#include <maf/messaging/client-server/ipc/LocalIPCReceiverImpl.h>

namespace maf {
namespace messaging {
namespace ipc {

LocalIPCReceiver::LocalIPCReceiver() {
  _impl = std::make_unique<LocalIPCReceiverImpl>();
}

LocalIPCReceiver::~LocalIPCReceiver() {}

bool LocalIPCReceiver::init(const Address &address) {
  return _impl->init(address);
}

bool LocalIPCReceiver::start() { return _impl->start(); }

void LocalIPCReceiver::stop() { _impl->stop(); }

bool LocalIPCReceiver::running() const { return _impl->running(); }

void LocalIPCReceiver::deinit()
{
    _impl->deinit();
}

const Address &LocalIPCReceiver::address() const { return _impl->address(); }

void LocalIPCReceiver::setObserver(BytesComeObserver *observer) {
  _impl->setObserver(
      [observer](auto &&bytes) { observer->onBytesCome(std::move(bytes)); });
}

} // namespace ipc
} // namespace messaging
} // namespace maf
