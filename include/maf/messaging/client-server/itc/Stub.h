#pragma once

#include "ParamTrait.h"
#include <maf/messaging/client-server/BasicStub.h>

namespace maf {
namespace messaging {
namespace itc {

using Stub = BasicStub<ParamTrait>;
using StubPtr = std::shared_ptr<Stub>;
using ExecutorIFPtr = Stub::ExecutorIFPtr;
template <class CSParam> using Request = Stub::Request<CSParam>;

inline std::shared_ptr<Stub>
createStub(const ServiceID &sid, Stub::ExecutorIFPtr executor = {}) {
  return Stub::createStub("itc.messaging.maf", {}, sid, std::move(executor));
}

} // namespace inapp
} // namespace messaging
} // namespace maf
