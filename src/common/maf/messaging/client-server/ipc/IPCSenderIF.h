#pragma once

#include <maf/messaging/client-server/Address.h>
#include <maf/messaging/client-server/CSStatus.h>
#include <maf/utils/serialization/ByteArray.h>

namespace maf {
namespace messaging {
namespace ipc {

class IPCSenderIF {
public:
  virtual ~IPCSenderIF() = default;
  virtual ActionCallStatus send(const srz::ByteArray &ba,
                                const Address &destination) = 0;
  virtual Availability
  checkReceiverStatus(const Address &destination) const = 0;
};

} // namespace ipc
} // namespace messaging
} // namespace maf
