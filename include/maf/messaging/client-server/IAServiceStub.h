#pragma once

#include "QueueingServiceStub.h"
#include "DefaultMessageTrait.h"

namespace maf {
namespace messaging {
namespace inapp {

using ServiceStub               = QueueingServiceStub<DefaultMessageTrait>;
using Request                   = QueuedRequest<DefaultMessageTrait>;

static std::shared_ptr<ServiceStub> createStub(const ServiceID& sid)
{
    return ServiceStub::createStub("app_internal", {}, sid);
}

}
}
}

