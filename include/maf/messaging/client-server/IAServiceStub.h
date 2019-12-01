#pragma once

#include "QueueingServiceStub.h"
#include "DefaultMessageTrait.h"

namespace maf {
namespace messaging {
namespace inapp {

using ServiceStub        = QueueingServiceStub<DefaultMessageTrait>;

using ServiceStubPtr     = std::shared_ptr<ServiceStub>;

template<class CSParam>
using Request            = QueuedRequest<DefaultMessageTrait, CSParam>;

template<class CSParam>
using RequestPtrType     = std::shared_ptr<Request<CSParam>>;

static inline std::shared_ptr<ServiceStub> createStub(const ServiceID& sid)
{
    return ServiceStub::createStub(
        "app_internal",
        {},
        sid
        );
}

}
}
}

