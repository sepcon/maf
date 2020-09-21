#pragma once

#include <maf/messaging/client-server/BasicProxy.h>

#include "ParamTrait.h"

namespace maf {
namespace messaging {
namespace itc {

using Proxy = BasicProxy<ParamTrait>;
using ProxyPtr = std::shared_ptr<Proxy>;
using ExecutorIFPtr = Proxy::ExecutorIFPtr;
using ServiceStatusObserverPtr = Proxy::ServiceStatusObserverPtr;
template <class CSParam>
using Response = Proxy::Response<CSParam>;

static inline constexpr auto connectionType = "itc.messaging.maf";

inline ProxyPtr createProxy(const ServiceID &sid, ExecutorIFPtr executor = {},
                            ServiceStatusObserverPtr statusObsv = {}) {
  return Proxy::createProxy(connectionType, {}, sid, std::move(executor),
                            std::move(statusObsv));
}

}  // namespace itc
}  // namespace messaging
}  // namespace maf
