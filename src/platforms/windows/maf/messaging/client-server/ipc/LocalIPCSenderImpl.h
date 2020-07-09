#pragma once

#include "NamedPipeSenderBase.h"

namespace maf {
namespace messaging {
namespace ipc {

class LocalIPCSenderImpl : public NamedPipeSenderBase {
public:
  LocalIPCSenderImpl();
  ~LocalIPCSenderImpl();
  ActionCallStatus send(const maf::srz::ByteArray &ba,
                        const Address &destination);

private:
  OVERLAPPED oOverlap_;
};

} // namespace ipc
} // namespace messaging
} // namespace maf
