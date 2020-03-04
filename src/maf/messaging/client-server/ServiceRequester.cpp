#include <maf/messaging/client-server/ServiceRequester.h>
#include "ServiceRequesterImpl.h"

namespace maf {
namespace messaging {

ServiceRequester::ServiceRequester(
    const ServiceID& sid,
    std::weak_ptr<ClientInterface> client
    ): _pImpl {std::make_unique<ServiceRequesterImpl>(sid, std::move(client)) }
{
    setServiceID(sid);
}

ServiceRequester::~ServiceRequester() = default;

Availability ServiceRequester::serviceStatus() const
{
    return _pImpl->serviceStatus();
}

RegID ServiceRequester::registerStatus(
    const OpID& propertyID,
    CSMessageContentHandlerCallback callback,
    ActionCallStatus* callStatus
    )
{
    return _pImpl->registerStatus(
        propertyID,
        std::move(callback),
        callStatus
        );
}

RegID ServiceRequester::registerSignal(
    const OpID& propertyID,
    CSMessageContentHandlerCallback callback,
    ActionCallStatus* callStatus
    )
{
    return _pImpl->registerSignal(
        propertyID,
        std::move(callback),
        callStatus
        );
}

ActionCallStatus ServiceRequester::unregisterBroadcast(const RegID& regID)
{
    return _pImpl->unregisterBroadcast(regID);
}

ActionCallStatus ServiceRequester::unregisterBroadcastAll(const OpID& propertyID)
{
    return _pImpl->unregisterBroadcastAll(propertyID);
}

RegID ServiceRequester::sendRequestAsync(
    const OpID& opID,
    const CSMsgContentBasePtr &msgContent,
    CSMessageContentHandlerCallback callback,
    ActionCallStatus* callStatus
    )
{
    return _pImpl->sendRequestAsync(
        opID,
        msgContent,
        std::move(callback),
        callStatus
        );
}

CSMsgContentBasePtr ServiceRequester::getStatus(
    const OpID& propertyID,
    RequestTimeoutMs timeout,
    ActionCallStatus* callStatus
    )
{
    return _pImpl->getStatus(propertyID, timeout, callStatus);
}


CSMsgContentBasePtr ServiceRequester::sendRequest(
    const OpID& opID,
    const CSMsgContentBasePtr &msgContent,
    RequestTimeoutMs timeout,
    ActionCallStatus* callStatus
    )
{
    return _pImpl->sendRequest(
        opID,
        msgContent,
        timeout,
        callStatus
        );
}

void ServiceRequester::abortAction(
    const RegID &regID,
    ActionCallStatus* callStatus
    )
{
    return _pImpl->abortAction(
        regID,
        callStatus
        );
}

void ServiceRequester::addServiceStatusObserver(
    std::weak_ptr<ServiceStatusObserverInterface> serviceStatusObserver
    )
{
    _pImpl->addServiceStatusObserver(std::move(serviceStatusObserver));
}

void ServiceRequester::removeServiceStatusObserver(
    const std::weak_ptr<ServiceStatusObserverInterface> &serviceStatusObserver
    )
{
    _pImpl->addServiceStatusObserver(serviceStatusObserver);
}

bool ServiceRequester::onIncomingMessage(const CSMessagePtr &csMsg)
{
    return _pImpl->onIncomingMessage(csMsg);
}

void ServiceRequester::onServerStatusChanged(
    Availability oldStatus,
    Availability newStatus
    )
{
    _pImpl->onServerStatusChanged(
        oldStatus,
        newStatus
        );
}

void ServiceRequester::onServiceStatusChanged(
    const ServiceID& sid,
    Availability oldStatus,
    Availability newStatus
    )
{
    _pImpl->onServiceStatusChanged(sid, oldStatus, newStatus);
}


} // messaging
} // maf
