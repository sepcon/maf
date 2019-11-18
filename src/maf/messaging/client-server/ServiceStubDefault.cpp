#include <maf/messaging/client-server/ServiceStubDefault.h>
#include <maf/logging/Logger.h>

namespace maf {
namespace messaging {

using logging::Logger;

void ServiceStubDefault::onClientRequest(
    const std::shared_ptr<RequestInterface> &request
    )
{
    switch(request->getOperationCode())
    {
    case OpCode::StatusGet:
        handleStatusGetRequest(request);
        break;
    case OpCode::Register:
        updateLatestStatus(request);
        break;
    case OpCode::Request:
        handleRequest(request);
        break;
    default:
        break;
    }
}

ActionCallStatus ServiceStubDefault::setStatus(
    OpID propertyID,
    const CSMsgContentBasePtr &newStatus
    )
{
    if(newStatus)
    {
        _propertyMap.atomic()->insert_or_assign(
            propertyID,
            newStatus
            );

        return _MyBase::setStatus(propertyID, newStatus);
    }
    else
    {
        return ActionCallStatus::InvalidParam;
    }
}

CSMsgContentBasePtr ServiceStubDefault::getStatus(OpID propertyID)
{
    std::lock_guard lock(_propertyMap);
    if(auto itStatus = _propertyMap->find(propertyID);
        itStatus != _propertyMap->end())
    {
        return itStatus->second;
    }
    else
    {
        return {};
    }
}

bool ServiceStubDefault::setRequestHandler(
    OpID opID,
    RequestHandlerFunction handlerFunction
    )
{
    if(handlerFunction)
    {
        (*_requestHandlerMap.atomic())[opID] = std::move(handlerFunction);
    }
    else
    {
        Logger::error(
            "Trying to set empty function as handler for OpID ",
            opID
            );
    }
    return false;
}

void ServiceStubDefault::updateLatestStatus(const RequestPtr &request)
{
    auto currentStatus = getStatus(request->getOperationID());
    if(!currentStatus)
    {
        logging::Logger::warn(
            "status data of ID ",
            request->getOperationID(),
            " hasn't been set yet!"
            );
    }
    request->respond(currentStatus);
}

void ServiceStubDefault::handleStatusGetRequest(const RequestPtr& request)
{
    if(!handleRequest(request))
    {
        request->respond(
            getStatus(request->getOperationID())
            );
    }
}

bool ServiceStubDefault::handleRequest(const RequestPtr &request)
{
    auto opID = request->getOperationID();
    std::unique_lock lock(_requestHandlerMap);

    if(auto itHandler = _requestHandlerMap->find(opID);
        itHandler != _requestHandlerMap->end())
    {
        auto handlerFunction = itHandler->second;
        lock.unlock();
        handlerFunction(request);
        return true;
    }
    else
    {
        Logger::error("Not found handler for request ID ", opID);
    }

    return false;
}

ServiceStubDefault::ServiceStubDefault(
    ServiceID sid,
    std::weak_ptr<ServerInterface> server
    ) : ServiceProvider(sid, std::move(server), this)
{
}



} // messaging
} // maf
