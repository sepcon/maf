#pragma once

#include <maf/messaging/client-server/SerializableMessageTrait.h>
#include <maf/messaging/client-server/QueueingServiceProxy.h>

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

using ServiceProxy      = QueueingServiceProxy<SerializableMessageTrait>;
using ServiceProxyPtr   = std::shared_ptr<ServiceProxy>;

template<class CSParam>
using ResponseType      = ServiceProxy::ResponseType<CSParam>;

template<class CSParam>
using ResponsePtrType   = ServiceProxy::ResponsePtrType<CSParam>;

static inline std::shared_ptr<ServiceProxy> createProxy(
    const Address& addr,
    const ServiceID& sid
    )
{
    return ServiceProxy::createProxy(
        "local_ipc",
        addr,
        sid
        );
}

}
} // ipc
} // messaging
} // maf
