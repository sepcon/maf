#pragma once

#include <maf/messaging/client-server/QueueingServiceStub.h>
#include <maf/messaging/client-server/SerializableMessageTrait.h>

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

using ServiceStub               = QueueingServiceStub<SerializableMessageTrait>;
using Request                   = QueuedRequest<SerializableMessageTrait>;

static std::shared_ptr<ServiceStub> createStub(const Address& addr, const ServiceID& sid)
{
    return ServiceStub::createStub("local_ipc", addr, sid);
}

} // local
} // ipc
} // messaging
} // maf

