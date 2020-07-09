#pragma once

#include <maf/messaging/client-server/BasicProxy.h>

#include "ParamTrait.h"

namespace maf {
namespace messaging {
namespace itc {

using Proxy = BasicProxy<ParamTrait>;
using ProxyPtr = std::shared_ptr<Proxy>;
using ExecutorPtr = Proxy::ExecutorPtr;
using ServiceStatusObserverPtr = Proxy::ServiceStatusObserverPtr;
template <class CSParam>
using Response = Proxy::Response<CSParam>;

inline ProxyPtr createProxy(const ServiceID &sid, ExecutorPtr executor = {},
                            ServiceStatusObserverPtr statusObsv = {}) {
  return Proxy::createProxy("itc.messaging.maf", {}, sid, std::move(executor),
                            std::move(statusObsv));
}

}  // namespace itc
}  // namespace messaging
}  // namespace maf
