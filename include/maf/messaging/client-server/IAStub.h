#pragma once

#include "DefaultParamTrait.h"
#include "Stub.h"

namespace maf {
namespace messaging {
namespace inapp {

using Stub = Stub<DefaultParamTrait>;
using StubPtr = std::shared_ptr<Stub>;
using ExecutorPtr = Stub::ExecutorPtr;
template <class CSParam> using Request = RequestT<DefaultParamTrait, CSParam>;

static inline std::shared_ptr<Stub>
createStub(const ServiceID &sid, Stub::ExecutorPtr executor = {}) {
  return Stub::createStub("app_internal", {}, sid, std::move(executor));
}

} // namespace inapp
} // namespace messaging
} // namespace maf
