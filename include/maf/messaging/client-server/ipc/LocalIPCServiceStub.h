#pragma once

#include <maf/messaging/client-server/QueueingServiceStub.h>
#include <maf/messaging/client-server/SerializableMessageTrait.h>

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

using ClientRequestMessage      = ClientRequestMessage<SerializableMessageTrait>;
using ServiceStub               = QueueingServiceStub<SerializableMessageTrait>;
using Request                   = RequestT<SerializableMessageTrait>;

static std::shared_ptr<ServiceStub> createStub(const Address& addr, ServiceID sid)
{
    return ServiceStub::createStub("local_ipc", addr, sid);
}

} // local
} // ipc
} // messaging
} // maf

