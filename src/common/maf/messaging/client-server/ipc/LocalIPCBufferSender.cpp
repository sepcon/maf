#include "LocalIPCBufferSender.h"

#include <maf/messaging/client-server/ipc/LocalIPCBufferSenderImpl.h>

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

LocalIPCBufferSender::LocalIPCBufferSender() {
  _pImpl = std::make_unique<LocalIPCBufferSenderImpl>();
}

LocalIPCBufferSender::~LocalIPCBufferSender() {}

ActionCallStatus LocalIPCBufferSender::send(const srz::Buffer &ba,
                                            const Address &destination) {
  return _pImpl->send(ba, destination);
}

Availability LocalIPCBufferSender::checkReceiverStatus(
    const Address &destination) const {
  return _pImpl->checkReceiverStatus(destination);
}

}  // namespace local
}  // namespace ipc
}  // namespace messaging
}  // namespace maf
