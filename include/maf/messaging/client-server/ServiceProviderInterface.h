#pragma once

#include "CSStatus.h"
#include "ServiceMessageReceiver.h"

namespace maf {
namespace messaging {

class ServiceStubHandlerInterface;
class ServiceProviderInterface : public ServiceMessageReceiver
{
public:
    virtual ~ServiceProviderInterface()                                                 = default;
    virtual void startServing()                                                         = 0;
    virtual void stopServing()                                                          = 0;
    virtual void setStubHandler(ServiceStubHandlerInterface *stubHandler)               = 0;
    virtual ActionCallStatus respondToRequest(const CSMessagePtr &csMsg)                = 0;
    virtual ActionCallStatus setStatus(
        OpID propertyID,
        const CSMsgContentBasePtr& property
        )                                                                               = 0;
};

}
}
