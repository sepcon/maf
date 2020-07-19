#pragma once

#include <maf/messaging/client-server/Address.h>
#include <maf/messaging/client-server/CSStatus.h>
#include <maf/utils/serialization/Buffer.h>

namespace maf {
namespace messaging {
namespace ipc {

class BufferSenderIF {
 public:
  virtual ~BufferSenderIF() = default;
  virtual ActionCallStatus send(const srz::Buffer &ba,
                                const Address &destination) = 0;
  virtual Availability checkReceiverStatus(
      const Address &destination) const = 0;
};

}  // namespace ipc
}  // namespace messaging
}  // namespace maf
