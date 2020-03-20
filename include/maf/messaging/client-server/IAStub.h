#pragma once

#include "DefaultMessageTrait.h"
#include "Stub.h"

namespace maf {
namespace messaging {
namespace inapp {

using Stub = Stub<DefaultMessageTrait>;

using StubPtr = std::shared_ptr<Stub>;

template <class CSParam> using Request = RequestT<DefaultMessageTrait, CSParam>;

template <class CSParam> using RequestPtr = std::shared_ptr<Request<CSParam>>;

static inline std::shared_ptr<Stub>
createStub(const ServiceID &sid, Stub::ExecutorPtr executor = {}) {
  return Stub::createStub("app_internal", {}, sid, std::move(executor));
}
} // namespace inapp
} // namespace messaging
} // namespace maf
