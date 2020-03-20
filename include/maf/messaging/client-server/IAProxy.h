#pragma once

#include "DefaultMessageTrait.h"
#include "Proxy.h"

namespace maf {
namespace messaging {
namespace inapp {

using Proxy = Proxy<DefaultMessageTrait>;
using ProxyPtr = std::shared_ptr<Proxy>;

template <class CSParam> using ResponseType = Proxy::ResponseType<CSParam>;

template <class CSParam> using ResponsePtr = Proxy::ResponsePtr<CSParam>;

using ExecutorPtr = Proxy::ExecutorPtr;
using SVStatusObsvWptr = Proxy::SVStatusObsvWptr;

inline ProxyPtr createProxy(const ServiceID &sid, ExecutorPtr executor = {},
                            SVStatusObsvWptr statusObsv = {}) {
  return Proxy::createProxy("app_internal", {}, sid, std::move(executor),
                            std::move(statusObsv));
}

} // namespace inapp
} // namespace messaging
} // namespace maf
