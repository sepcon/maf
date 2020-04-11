#pragma once

#include <maf/messaging/client-server/SerializableParamTrait.h>
#include <maf/messaging/client-server/Stub.h>

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

using Stub = maf::messaging::Stub<SerializableParamTrait>;
using StubPtr = std::shared_ptr<Stub>;
using ExecutorPtr = Stub::ExecutorPtr;
template <typename InputType> using Request = Stub::Request<InputType>;

static inline std::shared_ptr<Stub>
createStub(const Address &addr, const ServiceID &sid,
           Stub::ExecutorPtr executor = {}) {
  return Stub::createStub("local_ipc", addr, sid, std::move(executor));
}

} // namespace local
} // namespace ipc
} // namespace messaging
} // namespace maf
