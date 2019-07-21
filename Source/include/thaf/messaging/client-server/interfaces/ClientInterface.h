#pragma once

#include "CSStatus.h"
#include "CSMessageReceiver.h"
#include "ServiceStatusObserverInterface.h"


namespace thaf {
namespace messaging {

class ServiceRequesterInterface;

class ClientInterface : public CSMessageReceiver, private ServiceStatusObserverInterface
{
public:
    virtual ~ClientInterface() = default;
    virtual DataTransmissionErrorCode sendMessageToServer(const CSMessagePtr& msg) = 0;
    virtual bool registerServiceRequester(const std::shared_ptr<ServiceRequesterInterface>& requester) = 0;
    virtual bool unregisterServiceRequester(const std::shared_ptr<ServiceRequesterInterface>& requester) = 0;
    virtual bool unregisterServiceRequester(ServiceID sid) = 0;
    virtual bool hasServiceRequester(ServiceID sid) = 0;
};

}
}
