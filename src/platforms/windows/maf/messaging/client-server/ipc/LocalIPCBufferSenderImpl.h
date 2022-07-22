#pragma once

#include "NamedPipeSenderBase.h"

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

class LocalIPCBufferSenderImpl : public NamedPipeSenderBase {
 public:
  ActionCallStatus send(const maf::srz::Buffer &ba, const Address &destination);

};

}  // namespace local
}  // namespace ipc
}  // namespace messaging
}  // namespace maf
