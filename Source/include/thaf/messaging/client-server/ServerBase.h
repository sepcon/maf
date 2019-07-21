#pragma once

#pragma once

#include "interfaces/ServerInterface.h"
#include "prv/ServiceManagement.h"


namespace thaf {
namespace messaging {

class ServerBase : public ServerInterface
{
public:
    // Drived class must provide implementation for this method
    DataTransmissionErrorCode sendMessageToClient(const CSMessagePtr& msg, const Address& addr = Address::INVALID_ADDRESS)  override = 0;
    virtual void notifyServiceStatusToClient(ServiceID sid, Availability oldStatus, Availability newStatus) = 0;
    bool registerServiceProvider(const std::shared_ptr<ServiceProviderInterface>& provider)  override;
    bool unregisterServiceProvider(const std::shared_ptr<ServiceProviderInterface>& Provider)  override;
    bool unregisterServiceProvider(ServiceID sid) override;
    bool hasServiceProvider(ServiceID sid) override;
    bool onIncomingMessage(const CSMessagePtr& csMsg) override;
    std::shared_ptr<ServiceProviderInterface> getServiceProvider(ServiceID sid);

protected:
    using Providers = SMList<ServiceProviderInterface>;
    Providers _providers;
};

} // messaging
} // thaf
