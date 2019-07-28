#pragma once

#include "interfaces/ClientInterface.h"
#include "interfaces/ServiceStatusObserverInterface.h"
#include "interfaces/ServiceRequesterInterface.h"
#include "prv/ServiceManagement.h"

namespace thaf {
namespace messaging {

class ClientBase : public ClientInterface
{
public:
    //Dervied class must provide implementation for this method
    DataTransmissionErrorCode sendMessageToServer(const CSMessagePtr& msg) override = 0;
    bool registerServiceRequester(const IServiceRequesterPtr& requester)  override;
    bool unregisterServiceRequester(const IServiceRequesterPtr& requester)  override;
    bool unregisterServiceRequester(ServiceID sid) override;
    void onServerStatusChanged(Availability oldStatus, Availability newStatus) override;
    void onServiceStatusChanged(ServiceID sid, Availability oldStatus, Availability newStatus) override;
    bool hasServiceRequester(ServiceID sid) override;
    IServiceRequesterPtr getServiceRequester(ServiceID sid) override;
    void init();
    void deinit();
protected:
    bool onIncomingMessage(const CSMessagePtr& msg) override;

    using Requesters = SMList<ServiceRequesterInterface>;
    Requesters _requesters;
};

} // messaging
} // thaf
