#include "LocalIPCSender.h"
#include <maf/messaging/client-server/ipc/LocalIPCSenderImpl.h>

namespace maf {
namespace messaging {
namespace ipc {

LocalIPCSender::LocalIPCSender() {
  _pImpl = std::make_unique<LocalIPCSenderImpl>();
}

LocalIPCSender::~LocalIPCSender() {}

ActionCallStatus LocalIPCSender::send(const srz::ByteArray &ba,
                                      const Address &destination) {
  return _pImpl->send(ba, destination);
}

Availability
LocalIPCSender::checkReceiverStatus(const Address &destination) const {
  return _pImpl->checkReceiverStatus(destination);
}

} // namespace ipc
} // namespace messaging
} // namespace maf
