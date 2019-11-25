#include <maf/messaging/client-server/ServiceRequester.h>
#include "ServiceRequesterImpl.h"

namespace maf {
namespace messaging {

ServiceRequester::ServiceRequester(const ServiceID& sid, std::weak_ptr<ClientInterface> client):
      _pImpl {std::make_unique<ServiceRequesterImpl>(sid, std::move(client)) }
{
    setServiceID(sid);
}

ServiceRequester::~ServiceRequester() = default;

Availability ServiceRequester::serviceStatus() const
{
    return _pImpl->serviceStatus();
}

RegID ServiceRequester::registerStatus(const OpID& propertyID, CSMessageContentHandlerCallback callback)
{
    return _pImpl->registerStatus(propertyID, std::move(callback));
}

RegID ServiceRequester::registerSignal(const OpID& propertyID, CSMessageContentHandlerCallback callback)
{
    return _pImpl->registerSignal(propertyID, std::move(callback));
}

void ServiceRequester::unregisterStatus(const RegID& regID)
{
    _pImpl->unregisterStatus(regID);
}

void ServiceRequester::unregisterStatusAll(const OpID& propertyID)
{
    _pImpl->unregisterStatusAll(propertyID);
}

RegID ServiceRequester::getStatusAsync(const OpID& propertyID, CSMessageContentHandlerCallback callback)
{
    return _pImpl->getStatusAsync(propertyID, std::move(callback));
}


RegID ServiceRequester::sendRequestAsync(const OpID& opID, const CSMsgContentBasePtr &msgContent, CSMessageContentHandlerCallback callback)
{
    return _pImpl->sendRequestAsync(opID, msgContent, std::move(callback));
}

CSMsgContentBasePtr ServiceRequester::getStatus(const OpID& propertyID, unsigned long maxWaitTimeMs)
{
    return _pImpl->getStatus(propertyID, maxWaitTimeMs);
}


CSMsgContentBasePtr ServiceRequester::sendRequest(const OpID& opID, const CSMsgContentBasePtr &msgContent, unsigned long maxWaitTimeMs)
{
    return _pImpl->sendRequest(opID, msgContent, maxWaitTimeMs);
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

void ServiceRequester::onServiceStatusChanged(const ServiceID& sid, Availability oldStatus, Availability newStatus)
{
    _pImpl->onServiceStatusChanged(sid, oldStatus, newStatus);
}


} // messaging
} // maf
