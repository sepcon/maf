#pragma once

#include <maf/messaging/client-server/SerializableMessageTrait.h>
#include <maf/messaging/client-server/Stub.h>

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

using Stub = maf::messaging::Stub<SerializableMessageTrait>;

using StubPtr = std::shared_ptr<Stub>;

template <typename InputType> using Request = Stub::Request<InputType>;

template <typename InputType> using RequestPtr = Stub::RequestPtr<InputType>;

static inline std::shared_ptr<Stub>
createStub(const Address &addr, const ServiceID &sid,
           Stub::ExecutorPtr executor = {}) {
  return Stub::createStub("local_ipc", addr, sid, std::move(executor));
}

} // namespace local
} // namespace ipc
} // namespace messaging
} // namespace maf
