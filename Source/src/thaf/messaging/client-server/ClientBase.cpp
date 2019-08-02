#include "thaf/messaging/client-server/ClientBase.h"
#include "thaf/messaging/client-server/ServiceRequesterInterface.h"
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
    auto lockSSMap(_serviceStatusMap.pa_lock());
    for(auto& service : *_serviceStatusMap)
    {
        service.second = Availability::Unavailable;
    }

    auto lockRequesters(_requesters.pa_lock());
    for(auto& requester : *_requesters)
    {
        requester->onServerStatusChanged(oldStatus, newStatus);
    }
}

void ClientBase::onServiceStatusChanged(ServiceID sid, Availability oldStatus, Availability newStatus)
{
    thafInfo("Client receives service status update from server: [" << sid << "-" << static_cast<int>(oldStatus) << "-" << static_cast<int>(newStatus));
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
        thafInfo("Receive Service status update from server: sid[" << msg->serviceID() << "]-status[" << msg->operationID() << "]");
        if(msg->operationID() == OpID_ServiceAvailable)
        {
            storeServiceStatus(msg->serviceID(), Availability::Available);
            onServiceStatusChanged(msg->serviceID(), Availability::Unavailable, Availability::Available);
        }
        else if(msg->operationID() == OpID_ServiceUnavailable)
        {
            storeServiceStatus(msg->serviceID(), Availability::Unavailable);
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

void ClientBase::storeServiceStatus(ServiceID sid, Availability status)
{
    auto lock(_serviceStatusMap.pa_lock());
    (*_serviceStatusMap)[sid] = status;
}

IServiceRequesterPtr ClientBase::getServiceRequester(ServiceID sid)
{
    return findByID(_requesters, sid);
}

Availability ClientBase::getServiceStatus(ServiceID sid)
{
    auto lock(_serviceStatusMap.pa_lock());
    auto itStatus = _serviceStatusMap->find(sid);
    if(itStatus != _serviceStatusMap->end())
    {
        return itStatus->second;
    }
    else
    {
        return Availability::Unavailable;
    }
}

void ClientBase::init()
{
    //TBD: add if needed
}

void ClientBase::deinit()
{
    {
        auto lock = _requesters.a_lock();
        _requesters->clear();
    }
    {
        auto lock = _serviceStatusMap.a_lock();
        _serviceStatusMap->clear();
    }
}

}
}
