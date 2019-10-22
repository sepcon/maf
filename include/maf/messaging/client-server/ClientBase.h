#pragma once

#include "ClientInterface.h"
#include "ServiceStatusObserverInterface.h"
#include "ServiceRequesterInterface.h"
#include "internal/CSShared.h"
#include <maf/utils/debugging/Debug.h>
#include <map>

namespace maf {
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
    Availability getServiceStatus(ServiceID sid) override;

    void init();
    void deinit();

protected:
    bool onIncomingMessage(const CSMessagePtr& msg) override;
    void storeServiceStatus(ServiceID sid, Availability status);
    using Requesters = SMList<ServiceRequesterInterface>;
    using ServiceStatusMap = nstl::Lockable<std::map<ServiceID, Availability>>;
    Requesters _requesters;
    ServiceStatusMap _serviceStatusMap;
};

} // messaging
} // maf
