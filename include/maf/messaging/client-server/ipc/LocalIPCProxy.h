#pragma once

#include <maf/messaging/client-server/Proxy.h>
#include <maf/messaging/client-server/SerializableMessageTrait.h>

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

using Proxy = Proxy<SerializableMessageTrait>;
using ProxyPtr = std::shared_ptr<Proxy>;

template <class CSParam> using ResponseType = Proxy::ResponseType<CSParam>;

template <class CSParam> using ResponsePtr = Proxy::ResponsePtr<CSParam>;

using ExecutorPtr = Proxy::ExecutorPtr;
using SVStatusObsvWptr = Proxy::SVStatusObsvWptr;

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
