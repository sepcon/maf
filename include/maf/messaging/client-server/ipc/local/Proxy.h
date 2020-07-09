#pragma once

#include <maf/messaging/client-server/BasicProxy.h>
#include <maf/messaging/client-server/ipc/local/ParamTrait.h>

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

using Proxy = BasicProxy<ParamTrait>;
using ProxyPtr = std::shared_ptr<Proxy>;
using ExecutorPtr = Proxy::ExecutorPtr;
using ServiceStatusObserverPtr = Proxy::ServiceStatusObserverPtr;
template <class Output>
using Response = Proxy::Response<Output>;

inline ProxyPtr createProxy(const Address &addr, const ServiceID &sid,
                            ExecutorPtr executor = {},
                            ServiceStatusObserverPtr statusObsv = {}) {
  return Proxy::createProxy("local.ipc.messaging.maf", addr, sid,
                            std::move(executor), std::move(statusObsv));
}

}  // namespace local
}  // namespace ipc
}  // namespace messaging
}  // namespace maf
