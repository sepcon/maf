#pragma once

#include "QueueingServiceProxy.h"
#include "IAMessageRouter.h"
#include "DefaultMessageTrait.h"

namespace maf {
namespace messaging {
namespace inapp {

using ServiceProxy = QueueingServiceProxy<DefaultMessageTrait>;
static std::shared_ptr<ServiceProxy> createProxy(const ServiceID& sid)
{
    return ServiceProxy::createProxy("app_internal", {}, sid);
}


}
} // messaging
} // maf
