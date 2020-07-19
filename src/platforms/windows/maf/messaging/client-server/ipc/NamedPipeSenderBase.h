#pragma once

#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/Address.h>
#include <maf/messaging/client-server/CSStatus.h>
#include <maf/utils/serialization/Buffer.h>

#include "PipeShared.h"

namespace maf {

namespace messaging {
namespace ipc {

class NamedPipeSenderBase {
 public:
  ~NamedPipeSenderBase() = default;
  ActionCallStatus send(const maf::srz::Buffer& /*ba*/,
                        const Address& /*destination*/) {
    MAF_LOGGER_ERROR(
        "Derived class must this function[NamedPipeSenderBase::send]");
    return ActionCallStatus::ReceiverUnavailable;
  }

  Availability checkReceiverStatus(const Address& receiverAddr) const {
    static thread_local Address receiverAddress_;
    static thread_local PipeNameType pipeName_;
    if (receiverAddress_ != receiverAddr) {
      receiverAddress_ = receiverAddr;
      pipeName_ = constructPipeName(receiverAddr);
    }
    return WaitNamedPipeA(pipeName_.c_str(), WAIT_DURATION_MAX)
               ? Availability::Available
               : Availability::Unavailable;
  }
};

}  // namespace ipc
}  // namespace messaging
}  // namespace maf
