#include <maf/messaging/client-server/ClientBase.h>
#include <maf/messaging/client-server/ServiceRequesterInterface.h>
#include <maf/logging/Logger.h>

namespace maf { using logging::Logger;
namespace messaging {

bool ClientBase::registerServiceRequester(const IServiceRequesterPtr &requester)
{
    bool isNewRequester = true;
    if( (isNewRequester = addIfNew(_requesters, requester)) )
    {
        std::lock_guard lock(_serviceStatusMap);
        auto itServiceID = _serviceStatusMap->find(requester->serviceID());
        if(itServiceID != _serviceStatusMap->end())
        {
            requester->onServiceStatusChanged(itServiceID->first, Availability::Unknown, itServiceID->second);
        }
    }
    return isNewRequester;

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
    if(newStatus != Availability::Available)
    {
        _serviceStatusMap.atomic()->clear();
    }
    {
        std::lock_guard lock(_requesters);
        for(auto& requester : *_requesters)
        {
            requester->onServerStatusChanged(oldStatus, newStatus);
        }
    }
}

void ClientBase::onServiceStatusChanged(ServiceID sid, Availability oldStatus, Availability newStatus)
{
    Logger::info("Client receives service status update from server: [" ,  sid ,  "-" ,  static_cast<int>(oldStatus) ,  "-" ,  static_cast<int>(newStatus));
    {
        (*_serviceStatusMap.atomic())[sid] = newStatus;
    }
    auto requester = findByID(_requesters, sid);
    if(requester)
    {
        requester->onServiceStatusChanged(sid, oldStatus, newStatus);
    }
    else
    {
        Logger::warn("There's no requester for this service id: " ,  sid);
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
        Logger::info("Receive Service status update from server: sid[" ,  msg->serviceID() ,  "]-status[" ,  msg->operationID() ,  "]");
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
    (*_serviceStatusMap.atomic())[sid] = status;
}

IServiceRequesterPtr ClientBase::getServiceRequester(ServiceID sid)
{
    return findByID(_requesters, sid);
}

Availability ClientBase::getServiceStatus(ServiceID sid)
{
    std::lock_guard lock(_serviceStatusMap);
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

bool ClientBase::init(const Address &, long long)
{
    return true;
}

bool ClientBase::deinit()
{
    _requesters.atomic()->clear();
    _serviceStatusMap.atomic()->clear();
    return true;
}

}
}
