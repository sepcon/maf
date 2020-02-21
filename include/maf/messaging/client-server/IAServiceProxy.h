#pragma once

#include "QueueingServiceProxy.h"
#include "DefaultMessageTrait.h"

namespace maf {
namespace messaging {
namespace inapp {

using ServiceProxy      = QueueingServiceProxy<DefaultMessageTrait>;
using ServiceProxyPtr   = std::shared_ptr<ServiceProxy>;

template<class CSParam>
using ResponseType      = ServiceProxy::ResponseType<CSParam>;

template<class CSParam>
using ResponsePtrType   = ServiceProxy::ResponsePtrType<CSParam>;

static inline std::shared_ptr<ServiceProxy> createProxy(const ServiceID& sid)
{
    return ServiceProxy::createProxy("app_internal", {}, sid);
}


}
} // messaging
} // maf
