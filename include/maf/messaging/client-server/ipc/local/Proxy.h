#pragma once

#include <maf/messaging/client-server/BasicProxy.h>

#include "ConnectionType.h"
#include "ParamTrait.h"

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

using Proxy = BasicProxy<ParamTrait>;
using ProxyPtr = std::shared_ptr<Proxy>;
using ExecutorIFPtr = Proxy::ExecutorIFPtr;
using ServiceStatusObserverPtr = Proxy::ServiceStatusObserverPtr;
template <class Output>
using Response = Proxy::Response<Output>;

inline ProxyPtr createProxy(const Address &addr, const ServiceID &sid,
                            ExecutorIFPtr executor = {},
                            ServiceStatusObserverPtr statusObsv = {}) {
  return Proxy::createProxy(connection_type, addr, sid, std::move(executor),
                            std::move(statusObsv));
}

}  // namespace local
}  // namespace ipc
}  // namespace messaging
}  // namespace maf
