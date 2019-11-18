#include <maf/messaging/client-server/ServiceProvider.h>
#include "ServiceProviderImpl.h"

namespace maf {
namespace messaging {

ServiceProvider::ServiceProvider(
    ServiceID sid,
    std::weak_ptr<ServerInterface> server,
    ServiceStubHandlerInterface *stubHandler
    )
{
    setServiceID(sid);
    _pImpl = std::make_unique<ServiceProviderImpl>(
        this,
        std::move(server),
        stubHandler
        );
}

ServiceProvider::~ServiceProvider()
{
    _pImpl->stopServing();
}

void ServiceProvider::setStubHandler(ServiceStubHandlerInterface *stubHandler)
{
    _pImpl->setStubHandler(stubHandler);
}

ActionCallStatus ServiceProvider::respondToRequest(const CSMessagePtr &csMsg)
{
    return _pImpl->respondToRequest(csMsg);
}

ActionCallStatus ServiceProvider::setStatus(
    OpID propertyID,
    const CSMsgContentBasePtr &property
    )
{
    return _pImpl->setStatus(propertyID, property);
}

void ServiceProvider::startServing()
{
    _pImpl->startServing();
}

void ServiceProvider::stopServing()
{
    _pImpl->stopServing();
}

bool ServiceProvider::onIncomingMessage(const CSMessagePtr &csMsg)
{
    return _pImpl->onIncomingMessage(csMsg);
}

} //messaging
} //maf
