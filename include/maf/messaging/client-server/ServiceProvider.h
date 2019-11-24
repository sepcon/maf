#pragma once

#include "internal/CSShared.h"
#include "ServiceProviderInterface.h"

namespace maf {
namespace messaging {

class ServerInterface;
class ServiceStubHandlerInterface;
struct ServiceProviderImpl;

class ServiceProvider :
    public ServiceProviderInterface,
    public std::enable_shared_from_this<ServiceProvider>
{
public:
    ServiceProvider(
        ServiceID sid,
        std::weak_ptr<ServerInterface> server
        );

    ~ServiceProvider() override;

    bool registerRequestHandler(
        OpID opID,
        RequestHandlerFunction handlerFunction
        ) override;

    bool unregisterRequestHandler( OpID opID ) override;

    ActionCallStatus respondToRequest(
        const CSMessagePtr &csMsg
        ) override;

    ActionCallStatus setStatus(
        OpID propertyID,
        const CSMsgContentBasePtr& property
        ) override;

    CSMsgContentBasePtr getStatus(OpID propertyID) override;

    void startServing() override;
    void stopServing() override;

private:
    bool onIncomingMessage(const CSMessagePtr& csMsg) override;
    std::unique_ptr<ServiceProviderImpl> _pImpl;

};

}
}
