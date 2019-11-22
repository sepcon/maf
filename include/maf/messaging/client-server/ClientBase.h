#pragma once

#include "ClientInterface.h"
#include "ServiceRequesterInterface.h"
#include <maf/threading/Lockable.h>
#include <map>

namespace maf {
namespace messaging {

class ClientBase : public ClientInterface, public std::enable_shared_from_this<ClientBase>
{
public:
    //Dervied class must provide implementation for this method
    ActionCallStatus sendMessageToServer(const CSMessagePtr& msg) override = 0;
    void onServerStatusChanged(Availability oldStatus, Availability newStatus) override;
    void onServiceStatusChanged(ServiceID sid, Availability oldStatus, Availability newStatus) override;
    bool hasServiceRequester(ServiceID sid) override;
    ServiceRequesterInterfacePtr getServiceRequester(ServiceID sid) override;
    Availability getServiceStatus(ServiceID sid) override;

    bool init(const Address& serverAddress, long long sersverMonitoringCycleMS) override;
    bool deinit() override;

protected:
    bool onIncomingMessage(const CSMessagePtr& msg) override;
    void storeServiceStatus(ServiceID sid, Availability status);

    using ServiceStatusMap  = threading::Lockable<std::map<ServiceID, Availability>>;
    using ProxyMap          = threading::Lockable<std::map<ServiceID, ServiceRequesterInterfacePtr>>;
    ProxyMap                _requestersMap;
    ServiceStatusMap        _serviceStatusMap;

};

} // messaging
} // maf
