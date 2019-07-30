#pragma once

#include "CSStatus.h"
#include "CSMessageReceiver.h"
#include "ServiceStatusObserverInterface.h"
#include "DomainUser.h"

namespace thaf {
namespace messaging {

class ServiceRequesterInterface;
using IServiceRequesterPtr = std::shared_ptr<ServiceRequesterInterface>;

class ClientInterface : public CSMessageReceiver, private ServiceStatusObserverInterface, public DomainUser
{
public:
    virtual ~ClientInterface() = default;
    virtual DataTransmissionErrorCode sendMessageToServer(const CSMessagePtr& msg) = 0;
    virtual bool registerServiceRequester(const IServiceRequesterPtr& requester) = 0;
    virtual bool unregisterServiceRequester(const IServiceRequesterPtr& requester) = 0;
    virtual bool unregisterServiceRequester(ServiceID sid) = 0;
    virtual bool hasServiceRequester(ServiceID sid) = 0;
    virtual IServiceRequesterPtr getServiceRequester(ServiceID sid) = 0;
};

}
}
