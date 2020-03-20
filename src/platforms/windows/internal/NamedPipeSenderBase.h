#pragma once

#include "PipeShared.h"
#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/Address.h>
#include <maf/messaging/client-server/CSStatus.h>
#include <maf/utils/serialization/ByteArray.h>

namespace maf {

namespace messaging {
namespace ipc {

class NamedPipeSenderBase {
public:
  ~NamedPipeSenderBase() = default;
  bool initConnection(const Address &addr) {
    if (addr.valid() && receiverAddress_ != addr) {
      receiverAddress_ = addr;
      pipeName_ = constructPipeName(addr);
      return true;
    } else {
      return false;
    }
  }
  ActionCallStatus send(const maf::srz::ByteArray & /*ba*/,
                        const Address & /*destination*/) {
    MAF_LOGGER_ERROR(
        "Derived class must this function[NamedPipeSenderBase::send]");
    return ActionCallStatus::ReceiverUnavailable;
  }
  const Address &receiverAddress() const { return receiverAddress_; }
  Availability checkReceiverStatus() const {
    return WaitNamedPipeA(pipeName_.c_str(), WAIT_DURATION_MAX)
               ? Availability::Available
               : Availability::Unavailable;
  }

protected:
  std::string pipeName_;
  Address receiverAddress_;
};

} // namespace ipc
} // namespace messaging
} // namespace maf
