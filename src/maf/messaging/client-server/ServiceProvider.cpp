#include <maf/messaging/client-server/ServiceProvider.h>
#include "ServiceProviderImpl.h"

namespace maf {
namespace messaging {

ServiceProvider::ServiceProvider(ServiceID sid,
    std::weak_ptr<ServerInterface> server)
{
    setServiceID(std::move(sid));
    _pImpl = std::make_unique<ServiceProviderImpl>(
        this,
        std::move(server)
        );
}

ServiceProvider::~ServiceProvider()
{
    _pImpl->stopServing();
}

bool ServiceProvider::registerRequestHandler(const OpID& opID, RequestHandlerFunction handlerFunction)
{
    return _pImpl->registerRequestHandler(opID, std::move(handlerFunction));
}

bool ServiceProvider::unregisterRequestHandler(const OpID& opID)
{
    return _pImpl->unregisterRequestHandler(opID);
}


ActionCallStatus ServiceProvider::respondToRequest(const CSMessagePtr &csMsg)
{
    return _pImpl->respondToRequest(csMsg);
}

ActionCallStatus ServiceProvider::setStatus(
    const OpID& propertyID,
    const CSMsgContentBasePtr &property
    )
{
    return _pImpl->setStatus(propertyID, property);
}

ActionCallStatus ServiceProvider::broadcastSignal(
    const OpID& signalID,
    const CSMsgContentBasePtr& signal
    )
{
    return _pImpl->broadcastSignal(signalID, signal);
}

CSMsgContentBasePtr ServiceProvider::getStatus(const OpID& propertyID)
{
    return _pImpl->getStatus(propertyID);
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
