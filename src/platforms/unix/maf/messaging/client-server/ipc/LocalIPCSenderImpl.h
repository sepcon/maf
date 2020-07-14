#pragma once

#include "SocketShared.h"
#include <maf/messaging/client-server/CSStatus.h>
#include <maf/utils/serialization/ByteArray.h>

namespace maf {
namespace messaging {
namespace ipc {

class LocalIPCSenderImpl {
public:
  using Buffer = maf::srz::ByteArray;

  ActionCallStatus send(const Buffer &payload, const Address &destination);
  ActionCallStatus send(const Buffer &payload, const SocketPath &sockpath);
  Availability checkReceiverStatus(const Address &destination) const;
};

} // namespace ipc
} // namespace messaging
} // namespace maf
