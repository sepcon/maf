#pragma once

#include <maf/messaging/client-server/BasicStub.h>
#include <maf/messaging/client-server/ipc/local/ParamTrait.h>

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

using Stub = BasicStub<ParamTrait>;
using StubPtr = std::shared_ptr<Stub>;
using ExecutorPtr = Stub::ExecutorPtr;
template <class Input> using Request = Stub::Request<Input>;

inline std::shared_ptr<Stub> createStub(const Address &addr,
                                        const ServiceID &sid,
                                        Stub::ExecutorPtr executor = {}) {
  return Stub::createStub("local.ipc.messaging.maf", addr, sid,
                          std::move(executor));
}

} // namespace local
} // namespace ipc
} // namespace messaging
} // namespace maf
