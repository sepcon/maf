#pragma once

#include "internal/CSShared.h"
#include "ServiceProviderInterface.h"

namespace maf {
namespace messaging {

class ServerInterface;
class ServiceStubHandlerInterface;
struct ServiceProviderImpl;

class ServiceProvider : public ServiceProviderInterface, public std::enable_shared_from_this<ServiceProvider>
{
public:
    ServiceProvider(
        ServiceID sid,
        std::weak_ptr<ServerInterface> server,
        ServiceStubHandlerInterface* stubHandler = nullptr
        );

    ~ServiceProvider() override;

    void setStubHandler(ServiceStubHandlerInterface *stubHandler) override;

    ActionCallStatus respondToRequest(const CSMessagePtr &csMsg) override;

    ActionCallStatus setStatus(
        OpID propertyID,
        const CSMsgContentBasePtr& property
        ) override;

    void startServing() override;
    void stopServing() override;

private:
    bool onIncomingMessage(const CSMessagePtr& csMsg) override;
    std::unique_ptr<ServiceProviderImpl> _pImpl;

};

}
}
