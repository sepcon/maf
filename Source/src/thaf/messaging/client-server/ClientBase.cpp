#include "thaf/messaging/client-server/ClientBase.h"
#include "thaf/messaging/client-server/interfaces/ServiceRequesterInterface.h"
#include "thaf/utils/debugging/Debug.h"

namespace thaf {
namespace messaging {

bool ClientBase::registerServiceRequester(const IServiceRequesterPtr &requester)
{
    return addIfNew(_requesters, requester);
}

bool ClientBase::unregisterServiceRequester(const IServiceRequesterPtr &requester)
{
    return remove(_requesters, requester);
}

bool ClientBase::unregisterServiceRequester(ServiceID sid)
{
    return removeByID(_requesters, sid);
}

void ClientBase::onServerStatusChanged(Availability oldStatus, Availability newStatus)
{
    auto lock(_requesters.pa_lock());
    for(auto& requester : *_requesters)
    {
        requester->onServerStatusChanged(oldStatus, newStatus);
    }
}

void ClientBase::onServiceStatusChanged(ServiceID sid, Availability oldStatus, Availability newStatus)
{
    auto requester = findByID(_requesters, sid);
    if(requester)
    {
        requester->onServiceStatusChanged(sid, oldStatus, newStatus);
    }
    else
    {
        thafWarn("There's no requester for this service id: " << sid);
    }
}

bool ClientBase::hasServiceRequester(ServiceID sid)
{
    return hasItemWithID(_requesters, sid);
}

bool ClientBase::onIncomingMessage(const CSMessagePtr& msg)
{
    if(msg->operationCode() == OpCode::ServiceStatusUpdate && msg->serviceID() != ServiceIDInvalid)
    {
        if(msg->operationID() == OpID_ServiceAvailable)
        {
            onServiceStatusChanged(msg->serviceID(), Availability::Unavailable, Availability::Available);
        }
        else if(msg->operationID() == OpID_ServiceUnavailable)
        {
            onServiceStatusChanged(msg->serviceID(), Availability::Available, Availability::Unavailable);
        }
        return true;
    }
    else
    {
        auto requester = findByID(_requesters, msg->serviceID());
        if(requester)
        {
            return requester->onIncomingMessage(msg);
        }
        return false;
    }
}

IServiceRequesterPtr ClientBase::getServiceRequester(ServiceID sid)
{
    return findByID(_requesters, sid);
}

void ClientBase::init()
{
    //TBD: add if needed
}

void ClientBase::deinit()
{
    auto lock = _requesters.a_lock();
    _requesters->clear();
}

}
}
