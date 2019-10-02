#include <maf/messaging/client-server/ServiceProxyBase.h>
#include "ServiceProxyBaseImpl.h"

namespace maf {
namespace messaging {

ServiceProxyBase::ServiceProxyBase(ServiceID sid, ClientInterface *client) :
    _pi{new ServiceProxyBaseImpl(sid, client)} { setServiceID(sid); }

ServiceProxyBase::~ServiceProxyBase()
{ delete _pi; }

RegID ServiceProxyBase::sendStatusChangeRegister(OpID propertyID, CSMessageHandlerCallback callback)
{ return _pi->sendStatusChangeRegister(propertyID, std::move(callback)); }

void ServiceProxyBase::sendStatusChangeUnregister(RegID regID)
{ _pi->sendStatusChangeUnregister(regID); }

void ServiceProxyBase::sendStatusChangeUnregisterAll(OpID propertyID)
{ _pi->sendStatusChangeUnregisterAll(propertyID); }

RegID ServiceProxyBase::sendRequest(const CSMsgContentPtr &msgContent, CSMessageHandlerCallback callback)
{ return _pi->sendRequest(msgContent, std::move(callback)); }

void ServiceProxyBase::sendAbortRequest(const RegID &regID)
{ _pi->sendAbortRequest(regID); }

void ServiceProxyBase::sendAbortSyncRequest(const RegID &regID)
{ _pi->sendAbortSyncRequest(regID); }

bool ServiceProxyBase::sendRequestSync(const CSMsgContentPtr &msgContent, CSMessageHandlerCallback callback, unsigned long maxWaitTimeMs)
{ return _pi->sendRequestSync(msgContent, std::move(callback), maxWaitTimeMs); }

CSMessagePtr ServiceProxyBase::sendRequestSync(const CSMsgContentPtr &msgContent, unsigned long maxWaitTimeMs)
{  return _pi->sendRequestSync(msgContent, maxWaitTimeMs); }

bool ServiceProxyBase::onIncomingMessage(const CSMessagePtr &csMsg)
{
    if(csMsg->serviceID() == serviceID())
    {
        return _pi->onIncomingMessage(csMsg);
    }
    else
    {
        return false;
    }
}

void ServiceProxyBase::onServerStatusChanged(Availability oldStatus, Availability newStatus)
{ _pi->onServerStatusChanged(oldStatus, newStatus); }

void ServiceProxyBase::onServiceStatusChanged(ServiceID sid, Availability oldStatus, Availability newStatus)
{ _pi->onServiceStatusChanged(sid, oldStatus, newStatus); }

ClientInterface *ServiceProxyBase::getClient()
{ return _pi->getClient(); }

}
}

