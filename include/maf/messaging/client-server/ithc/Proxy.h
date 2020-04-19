#ifndef MAF_MESSAGING_CLIENT_SERVER_ITHC_PROXY_H
#define MAF_MESSAGING_CLIENT_SERVER_ITHC_PROXY_H

#include "ParamTrait.h"
#include <maf/messaging/client-server/BasicProxy.h>

namespace maf {
namespace messaging {
namespace ithc {

using Proxy = BasicProxy<ParamTrait>;
using ProxyPtr = std::shared_ptr<Proxy>;
using ExecutorPtr = Proxy::ExecutorPtr;
using SVStatusObsvWptr = Proxy::SVStatusObsvWptr;
template <class CSParam> using Response = Proxy::Response<CSParam>;


inline ProxyPtr createProxy(const ServiceID &sid, ExecutorPtr executor = {},
                            SVStatusObsvWptr statusObsv = {}) {
  return Proxy::createProxy("ithc.messaging.maf", {}, sid, std::move(executor),
                            std::move(statusObsv));
}

} // namespace inapp
} // namespace messaging
} // namespace maf

#endif // MAF_MESSAGING_CLIENT_SERVER_ITHC_PROXY_H
