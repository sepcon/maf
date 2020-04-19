#pragma once

#include <maf/messaging/client-server/Address.h>
#include <maf/messaging/client-server/CSStatus.h>
#include <maf/utils/serialization/ByteArray.h>

namespace maf {
namespace messaging {
namespace ipc {

class IPCSender {
public:
  virtual ~IPCSender() = default;
  virtual bool initConnection(const Address &) = 0;
  virtual ActionCallStatus send(const srz::ByteArray &ba,
                                const Address &destination) = 0;
  virtual Availability checkReceiverStatus() const = 0;
  virtual const Address &receiverAddress() const = 0;
  //    virtual void stop() = 0;
};

} // namespace ipc
} // namespace messaging
} // namespace maf
