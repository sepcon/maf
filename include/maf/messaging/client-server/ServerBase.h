#pragma once

#include "ServerInterface.h"
#include "ServiceProviderInterface.h"
#include "internal/CSShared.h"
#include <maf/utils/cppextension/Maf.mc.h>


namespace maf {
namespace messaging {

class ServerBase : public ServerInterface
{
public:
    // Drived class must provide implementation for this method
    DataTransmissionErrorCode sendMessageToClient(const CSMessagePtr& msg, const Address& addr)  override = 0;
    virtual void notifyServiceStatusToClient(ServiceID sid, Availability oldStatus, Availability newStatus) = 0;
    bool registerServiceProvider(const IServiceProviderPtr& provider)  override;
    bool unregisterServiceProvider(const IServiceProviderPtr& Provider)  override;
    bool unregisterServiceProvider(ServiceID sid) override;
    bool hasServiceProvider(ServiceID sid) override;
    IServiceProviderPtr getServiceProvider(ServiceID sid) override;

    void init();
    void deinit();

protected:
    bool onIncomingMessage(const CSMessagePtr& csMsg) override;

    using Providers = SMList<ServiceProviderInterface>;
    Providers _providers;
};


} // messaging
} // maf
