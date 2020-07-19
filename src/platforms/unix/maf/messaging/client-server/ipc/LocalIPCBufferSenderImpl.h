#pragma once

#include <maf/messaging/client-server/CSStatus.h>
#include <maf/utils/serialization/Buffer.h>

#include "SocketShared.h"

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

class LocalIPCBufferSenderImpl {
 public:
  using Buffer = maf::srz::Buffer;

  ActionCallStatus send(const Buffer &payload, const Address &destination);
  ActionCallStatus send(const Buffer &payload, const SocketPath &sockpath);
  Availability checkReceiverStatus(const Address &destination) const;
};

}  // namespace local
}  // namespace ipc
}  // namespace messaging
}  // namespace maf
