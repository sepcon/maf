#pragma once

#include <maf/messaging/client-server/BasicStub.h>

#include "ConnectionType.h"
#include "ParamTrait.h"

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

using Stub = BasicStub<ParamTrait>;
using StubPtr = std::shared_ptr<Stub>;
using ExecutorIFPtr = Stub::ExecutorIFPtr;
template <class Input>
using Request = Stub::Request<Input>;

inline std::shared_ptr<Stub> createStub(const Address &addr,
                                        const ServiceID &sid,
                                        Stub::ExecutorIFPtr executor = {}) {
  return Stub::createStub(connection_type, addr, sid, std::move(executor));
}

}  // namespace local
}  // namespace ipc
}  // namespace messaging
}  // namespace maf
