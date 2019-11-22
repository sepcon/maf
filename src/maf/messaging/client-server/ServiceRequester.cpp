#include <maf/messaging/client-server/ServiceRequester.h>
#include "ServiceRequesterImpl.h"

namespace maf {
namespace messaging {

ServiceRequester::ServiceRequester(ServiceID sid, std::weak_ptr<ClientInterface> client):
      _pImpl {std::make_unique<ServiceRequesterImpl>(sid, std::move(client)) }
{
    setServiceID(sid);
}

ServiceRequester::~ServiceRequester() = default;

Availability ServiceRequester::serviceStatus() const
{
    return _pImpl->serviceStatus();
}

RegID ServiceRequester::registerStatus(OpID propertyID, CSMessageContentHandlerCallback callback)
{
    return _pImpl->registerStatus(propertyID, std::move(callback));
}

void ServiceRequester::unregisterStatus(const RegID& regID)
{
    _pImpl->unregisterStatus(regID);
}

void ServiceRequester::unregisterStatusAll(OpID propertyID)
{
    _pImpl->unregisterStatusAll(propertyID);
}

RegID ServiceRequester::getStatusAsync(OpID propertyID, CSMessageContentHandlerCallback callback)
{
    return _pImpl->getStatusAsync(propertyID, std::move(callback));
}


RegID ServiceRequester::requestActionAsync(OpID opID, const CSMsgContentBasePtr &msgContent, CSMessageContentHandlerCallback callback)
{
    return _pImpl->requestActionAsync(opID, msgContent, std::move(callback));
}

CSMsgContentBasePtr ServiceRequester::getStatus(OpID propertyID, unsigned long maxWaitTimeMs)
{
    return _pImpl->getStatus(propertyID, maxWaitTimeMs);
}


CSMsgContentBasePtr ServiceRequester::requestAction(OpID opID, const CSMsgContentBasePtr &msgContent, unsigned long maxWaitTimeMs)
{
    return _pImpl->requestAction(opID, msgContent, maxWaitTimeMs);
}

void ServiceRequester::abortAction(const RegID &regID)
{
    return _pImpl->abortAction(regID);
}

void ServiceRequester::addServiceStatusObserver(std::weak_ptr<ServiceStatusObserverInterface> serviceStatusObserver)
{
    _pImpl->addServiceStatusObserver(std::move(serviceStatusObserver));
}

void ServiceRequester::removeServiceStatusObserver(const std::weak_ptr<ServiceStatusObserverInterface> &serviceStatusObserver)
{
    _pImpl->addServiceStatusObserver(serviceStatusObserver);
}

bool ServiceRequester::onIncomingMessage(const CSMessagePtr &csMsg)
{
    return _pImpl->onIncomingMessage(csMsg);
}

void ServiceRequester::onServerStatusChanged(Availability oldStatus, Availability newStatus)
{
    _pImpl->onServerStatusChanged(oldStatus, newStatus);
}

void ServiceRequester::onServiceStatusChanged(ServiceID sid, Availability oldStatus, Availability newStatus)
{
    _pImpl->onServiceStatusChanged(sid, oldStatus, newStatus);
}


} // messaging
} // maf
