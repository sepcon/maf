#include <maf/messaging/client-server/ClientBase.h>
#include <maf/messaging/client-server/ServiceRequester.h>
#include <maf/logging/Logger.h>
#include <cassert>

namespace maf { using logging::Logger;
namespace messaging {

void ClientBase::onServerStatusChanged(
    Availability oldStatus,
    Availability newStatus
    )
{
    if(newStatus != Availability::Available)
    {
        _serviceStatusMap.atomic()->clear();
    }
    {
        std::lock_guard lock(_requestersMap);
        for(auto& [sid, proxy] : *_requestersMap)
        {
            proxy->onServerStatusChanged(oldStatus, newStatus);
        }
    }
}

void ClientBase::onServiceStatusChanged(
    ServiceID sid,
    Availability oldStatus,
    Availability newStatus
    )
{
    Logger::info(
        "Client receives service status update from server: [",
        sid,
        "-",
        static_cast<int>(oldStatus),  "-" ,  static_cast<int>(newStatus)
        );

    (*_serviceStatusMap.atomic())[sid] = newStatus;
    std::lock_guard lock(_requestersMap);
    auto itProxy = _requestersMap->find(sid);
    if(itProxy != _requestersMap->end())
    {
        itProxy->second->onServiceStatusChanged(sid, oldStatus, newStatus);
    }
    else
    {
        Logger::warn("There's no proxy for this service id: " ,  sid);
    }
}

bool ClientBase::hasServiceRequester(ServiceID sid)
{
    return _requestersMap.atomic()->count(sid) != 0;
}

bool ClientBase::onIncomingMessage(const CSMessagePtr& msg)
{
    if(msg->operationCode() == OpCode::ServiceStatusUpdate
        && msg->serviceID() != ServiceIDInvalid)
    {
        Logger::info("Receive Service status update from server: sid[" ,
                     msg->serviceID() ,
                     "]-status[" ,
                     msg->operationID() ,
                     "]"
                     );
        if(msg->operationID() == OpID_ServiceAvailable)
        {
            storeServiceStatus(msg->serviceID(), Availability::Available);
            onServiceStatusChanged(
                msg->serviceID(),
                Availability::Unavailable,
                Availability::Available
                );
        }
        else if(msg->operationID() == OpID_ServiceUnavailable)
        {
            storeServiceStatus(msg->serviceID(), Availability::Unavailable);
            onServiceStatusChanged(
                msg->serviceID(),
                Availability::Available,
                Availability::Unavailable
                );
        }
        return true;
    }
    else
    {
        std::lock_guard lock(_requestersMap);
        if(auto itProxy = _requestersMap->find(msg->serviceID());
            itProxy != _requestersMap->end())
        {
            return itProxy->second->onIncomingMessage(msg);
        }
        return false;
    }
}

void ClientBase::storeServiceStatus(ServiceID sid, Availability status)
{
    (*_serviceStatusMap.atomic())[sid] = status;
}

ServiceRequesterInterfacePtr ClientBase::getServiceRequester(ServiceID sid)
{
    std::lock_guard lock(_requestersMap);

    if(auto itRequester = _requestersMap->find(sid);
        itRequester != _requestersMap->end())
    {
        return itRequester->second;
    }
    else
    {
        assert(shared_from_this());
        ServiceRequesterInterfacePtr requester =
            std::make_shared<ServiceRequester>(sid, weak_from_this());
        std::lock_guard lock(_serviceStatusMap);
        auto itServiceStatus = _serviceStatusMap->find(sid);
        if(itServiceStatus != _serviceStatusMap->end())
        {
            requester->onServiceStatusChanged(
                itServiceStatus->first,
                Availability::Unknown,
                itServiceStatus->second
                );
        }
        _requestersMap->emplace(sid, requester);
        return requester;
    }
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
    _requestersMap.atomic()->clear();
    std::lock_guard lock(_serviceStatusMap);
    for(auto& [serviceID, status] : *_serviceStatusMap)
    {
        sendMessageToServer(
            createCSMessage(
                serviceID,
                OpIDInvalid,
                OpCode::UnregisterServiceStatus
                )
            );
    }
    _serviceStatusMap->clear();
    return true;
}

}
}
