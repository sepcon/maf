#pragma once

#include "ServerInterface.h"
#include "ServiceProviderInterface.h"
#include <maf/threading/Lockable.h>
#include <map>

namespace maf {
namespace messaging {

class ServerBase : public ServerInterface
{
public:
    // Drived class must provide implementation for this method
    ActionCallStatus sendMessageToClient(const CSMessagePtr& msg, const Address& addr)  override = 0;
    virtual void notifyServiceStatusToClient(ServiceID sid, Availability oldStatus, Availability newStatus) = 0;

    bool registerServiceProvider(const ServiceProviderInterfacePtr& provider)  override;
    bool unregisterServiceProvider(const ServiceProviderInterfacePtr& Provider)  override;
    bool unregisterServiceProvider(ServiceID sid) override;
    bool hasServiceProvider(ServiceID sid) override;

    virtual bool init(const Address& serverAddr) override;
    bool deinit() override;
protected:
    bool onIncomingMessage(const CSMessagePtr& csMsg) override;
    using ProviderMap = threading::Lockable<std::map<ServiceID, ServiceProviderInterfacePtr>>;
    ProviderMap _providers;
};


} // messaging
} // maf
