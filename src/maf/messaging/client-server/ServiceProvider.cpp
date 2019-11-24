#include <maf/messaging/client-server/ServiceProvider.h>
#include "ServiceProviderImpl.h"

namespace maf {
namespace messaging {

ServiceProvider::ServiceProvider(ServiceID sid,
    std::weak_ptr<ServerInterface> server)
{
    setServiceID(sid);
    _pImpl = std::make_unique<ServiceProviderImpl>(
        this,
        std::move(server)
        );
}

ServiceProvider::~ServiceProvider()
{
    _pImpl->stopServing();
}

bool ServiceProvider::registerRequestHandler(OpID opID, RequestHandlerFunction handlerFunction)
{
    return _pImpl->registerRequestHandler(opID, std::move(handlerFunction));
}

bool ServiceProvider::unregisterRequestHandler(OpID opID)
{
    return _pImpl->unregisterRequestHandler(opID);
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

CSMsgContentBasePtr ServiceProvider::getStatus(OpID propertyID)
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
