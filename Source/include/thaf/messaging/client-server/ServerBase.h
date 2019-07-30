#pragma once

#pragma once

#include "ServerInterface.h"
#include "ServiceProviderInterface.h"
#include "internal/CSShared.h"
#include "thaf/utils/cppextension/thaf.h"


namespace thaf {
namespace messaging {

class ServerBase : public ServerInterface
{
public:
    // Drived class must provide implementation for this method
    DataTransmissionErrorCode sendMessageToClient(const CSMessagePtr& msg, const Address& addr = Address::INVALID_ADDRESS)  override = 0;
    virtual void notifyServiceStatusToClient(ServiceID sid, Availability oldStatus, Availability newStatus) = 0;
    bool registerServiceProvider(const IServiceProviderPtr& provider)  override;
    bool unregisterServiceProvider(const IServiceProviderPtr& Provider)  override;
    bool unregisterServiceProvider(ServiceID sid) override;
    bool hasServiceProvider(ServiceID sid) override;
    IServiceProviderPtr getServiceProvider(ServiceID sid) override;

    void init();
    void deinit();

    template<class Stub, std::enable_if_t<std::is_base_of_v<ServiceProviderInterface, Stub>, bool> = true>
    std::shared_ptr<Stub> createStub(ServiceID sid) thaf_throws(std::rutime_error);

protected:
    bool onIncomingMessage(const CSMessagePtr& csMsg) override;

    using Providers = SMList<ServiceProviderInterface>;
    Providers _providers;
};

template<class Stub, std::enable_if_t<std::is_base_of_v<ServiceProviderInterface, Stub>, bool>>
std::shared_ptr<Stub> ServerBase::createStub(ServiceID sid)
{
    auto serviceProvider = getServiceProvider(sid);
    if(!serviceProvider)
    {
        serviceProvider.reset(new Stub(sid, this));
        if(!registerServiceProvider(serviceProvider))
        {
            //Error: there are more than one component trying to create Stub for one service ID
            std::runtime_error("Stub of service ID " + std::to_string(sid) + "has already taken care by another component!");
        }
    }
    else
    {
        throw std::runtime_error("Stub of service ID " + std::to_string(sid) + "has already taken care by another component!");
    }

    return std::static_pointer_cast<Stub>(serviceProvider);
}

} // messaging
} // thaf
