#ifndef MAF_MESSAGING_CLIENT_SERVER_IPC_LOCAL_PROXY_H
#define MAF_MESSAGING_CLIENT_SERVER_IPC_LOCAL_PROXY_H

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
using SVStatusObsvWptr = Proxy::SVStatusObsvWptr;
template <class Output> using Response = Proxy::Response<Output>;

inline ProxyPtr createProxy(const Address &addr, const ServiceID &sid,
                            ExecutorPtr executor = {},
                            SVStatusObsvWptr statusObsv = {}) {
  return Proxy::createProxy("local.ipc.messaging.maf", addr, sid,
                            std::move(executor), std::move(statusObsv));
}

} // namespace local
} // namespace ipc
} // namespace messaging
} // namespace maf

#endif // MAF_MESSAGING_CLIENT_SERVER_IPC_LOCAL_PROXY_H
