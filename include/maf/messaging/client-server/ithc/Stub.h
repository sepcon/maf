#pragma once

#include "ParamTrait.h"
#include <maf/messaging/client-server/BasicStub.h>

namespace maf {
namespace messaging {
namespace ithc {

using Stub = BasicStub<ParamTrait>;
using StubPtr = std::shared_ptr<Stub>;
using ExecutorPtr = Stub::ExecutorPtr;
template <class CSParam> using Request = Stub::Request<CSParam>;

inline std::shared_ptr<Stub>
createStub(const ServiceID &sid, Stub::ExecutorPtr executor = {}) {
  return Stub::createStub("ithc.messaging.maf", {}, sid, std::move(executor));
}

} // namespace inapp
} // namespace messaging
} // namespace maf
