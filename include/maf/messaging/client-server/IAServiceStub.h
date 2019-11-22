#pragma once

#include "QueueingServiceStub.h"
#include "DefaultMessageTrait.h"

namespace maf {
namespace messaging {
namespace inapp {

using ClientRequestMesasge      = ClientRequestMessage<DefaultMessageTrait>;
using ServiceStub               = QueueingServiceStub<DefaultMessageTrait>;
using Request                   = RequestT<DefaultMessageTrait>;

static std::shared_ptr<ServiceStub> createStub(ServiceID sid)
{
    return ServiceStub::createStub("app_internal", {}, sid);
}

}
}
}

