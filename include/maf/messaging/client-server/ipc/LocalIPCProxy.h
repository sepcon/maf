#pragma once

#include <maf/messaging/client-server/Proxy.h>
#include <maf/messaging/client-server/SerializableParamTrait.h>

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

using Proxy = Proxy<SerializableParamTrait>;
using ProxyPtr = std::shared_ptr<Proxy>;
using ExecutorPtr = Proxy::ExecutorPtr;
using SVStatusObsvWptr = Proxy::SVStatusObsvWptr;
template <class CSParam> using Response = Proxy::Response<CSParam>;


inline ProxyPtr createProxy(const Address &addr, const ServiceID &sid,
                            ExecutorPtr executor = {},
                            SVStatusObsvWptr statusObsv = {}) {
  return Proxy::createProxy("local_ipc", addr, sid, std::move(executor),
                            std::move(statusObsv));
}

} // namespace local
} // namespace ipc
} // namespace messaging
} // namespace maf
