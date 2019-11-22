#pragma once

#include <maf/messaging/client-server/SerializableMessageTrait.h>
#include <maf/messaging/client-server/QueueingServiceProxy.h>

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

using ServiceProxy = QueueingServiceProxy<SerializableMessageTrait>;
std::shared_ptr<ServiceProxy> createProxy(const Address& addr, ServiceID sid)
{
    return ServiceProxy::createProxy("local_ipc", addr, sid);
}

}
} // ipc
} // messaging
} // maf
