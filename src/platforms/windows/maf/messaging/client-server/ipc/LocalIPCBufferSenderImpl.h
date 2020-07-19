#pragma once

#include "NamedPipeSenderBase.h"

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

class LocalIPCBufferSenderImpl : public NamedPipeSenderBase {
 public:
  LocalIPCBufferSenderImpl();
  ~LocalIPCBufferSenderImpl();
  ActionCallStatus send(const maf::srz::Buffer &ba,
                        const Address &destination);

 private:
  OVERLAPPED oOverlap_;
};

}  // namespace local
}  // namespace ipc
}  // namespace messaging
}  // namespace maf
