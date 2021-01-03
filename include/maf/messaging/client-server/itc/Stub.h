#pragma once

#include <maf/messaging/client-server/BasicStub.h>

#include "ConnectionType.h"
#include "ParamTrait.h"

namespace maf {
namespace messaging {
namespace itc {

using Stub = BasicStub<ParamTrait>;
using StubPtr = std::shared_ptr<Stub>;
using ExecutorIFPtr = Stub::ExecutorIFPtr;
template <class CSParam>
using Request = Stub::Request<CSParam>;

inline std::shared_ptr<Stub> createStub(const ServiceID &sid = {},
                                        Stub::ExecutorIFPtr executor = {}) {
  return Stub::createStub(connection_type, {}, sid, std::move(executor));
}

}  // namespace itc
}  // namespace messaging
}  // namespace maf
