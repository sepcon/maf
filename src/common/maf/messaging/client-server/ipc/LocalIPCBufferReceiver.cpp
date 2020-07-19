#include "LocalIPCBufferReceiver.h"

#include <maf/messaging/client-server/ipc/LocalIPCBufferReceiverImpl.h>

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

LocalIPCBufferReceiver::LocalIPCBufferReceiver() {
  _impl = std::make_unique<LocalIPCBufferReceiverImpl>();
}

LocalIPCBufferReceiver::~LocalIPCBufferReceiver() {}

bool LocalIPCBufferReceiver::init(const Address &address) {
  return _impl->init(address);
}

bool LocalIPCBufferReceiver::start() { return _impl->start(); }

void LocalIPCBufferReceiver::stop() { _impl->stop(); }

bool LocalIPCBufferReceiver::running() const { return _impl->running(); }

void LocalIPCBufferReceiver::deinit() { _impl->deinit(); }

const Address &LocalIPCBufferReceiver::address() const {
  return _impl->address();
}

void LocalIPCBufferReceiver::setObserver(BytesComeObserver *observer) {
  _impl->setObserver(
      [observer](auto &&bytes) { observer->onBytesCome(std::move(bytes)); });
}

}  // namespace local
}  // namespace ipc
}  // namespace messaging
}  // namespace maf
