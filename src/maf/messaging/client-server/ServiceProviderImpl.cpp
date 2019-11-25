#include "ServiceProviderImpl.h"
#include <maf/messaging/client-server/ServiceProvider.h>
#include <maf/messaging/client-server/Request.h>
#include <maf/messaging/client-server/ServiceStubHandlerInterface.h>
#include <maf/messaging/client-server/ServerInterface.h>
#include <maf/logging/Logger.h>
#include <vector>


namespace maf {
namespace messaging {

using logging::Logger;

bool ServiceProviderImpl::onIncomingMessage(const CSMessagePtr &msg)
{
    Logger::info(
        "Received Incoming Message  : " ,
        "\n\tCode                   : " ,  msg->operationCode() ,
        "\n\tID                     : " ,  msg->operationID() ,
        "\n\tSenderAddress          : " ,  msg->sourceAddress().dump(2)
        );

    bool handled = true;
    switch (msg->operationCode())
    {
    case OpCode::Register:
        onStatusChangeRegister(msg);
        break;
    case OpCode::RegisterSignal:
        saveRegisterInfo(msg);
        break;
    case OpCode::UnRegister:
        onStatusChangeUnregister(msg);
        break;
    case OpCode::UnregisterServiceStatus:
        onClientGoesOff(msg);
        break;
    case OpCode::Request:
        onActionRequest(msg);
        break;
    case OpCode::StatusGet:
        onStatusGetRequest(msg);
        break;
    case OpCode::Abort:
        onAbortActionRequest(msg);
        break;
    default:
        handled = false;
        Logger::warn("Unhandled OpCode: " ,  msg->operationCode());
        break;
    }
    return handled;
}

ServiceProviderImpl::ServiceProviderImpl(ServiceProvider *holder,
    std::weak_ptr<ServerInterface> server) :
    _server(std::move(server)),
    _delegator(std::move(holder))
{
    assert(_server.lock() && "Server must not be null");
}


ServiceProviderImpl::~ServiceProviderImpl()
{
    removeAllRegisterInfo();
    invalidateAndRemoveAllRequests();
}


ActionCallStatus ServiceProviderImpl::respondToRequest(const CSMessagePtr& csMsg)
{
    if(auto request = pickOutRequestInfo(csMsg))
    {
        request->invalidate();
        return sendBackMessageToClient(csMsg);
    }
    else
    {
        return ActionCallStatus::FailedUnknown;
    }
}

ActionCallStatus ServiceProviderImpl::setStatus(
    const OpID&  propertyID,
    const CSMsgContentBasePtr& property
    )
{
    _propertyMap.atomic()->insert_or_assign(
        propertyID,
        property
        );

    return broadcast(propertyID, OpCode::Register, property);
}

ActionCallStatus ServiceProviderImpl::broadcastSignal(
    const OpID& signalID,
    const CSMsgContentBasePtr& signal
    )
{
    return broadcast(signalID, OpCode::RegisterSignal, signal);
}

ActionCallStatus ServiceProviderImpl::broadcast(
    const OpID&  propertyID,
    OpCode opCode,
    const CSMsgContentBasePtr& content
    )
{

    using AddressList = std::vector<Address>;
    bool success = false;
    AddressList addresses;

    { // locking _regEntriesMap block
        std::lock_guard lock(_regEntriesMap);
        for(const auto& [clientAddress, registeredPropertyIDs] : *_regEntriesMap)
        {
            if(registeredPropertyIDs.find(propertyID) != registeredPropertyIDs.end())
            {
                addresses.push_back(clientAddress);
            }
        }
    } // locking _regEntriesMap block

    if(addresses.empty())
    {
        Logger::warn("There's no register for property: " ,  propertyID);
    }
    else
    {
        auto csMsg = createCSMessage(
            _delegator->serviceID(),
            propertyID,
            opCode,
            RequestIDInvalid,
            content
            );

        auto trySendToDestinations =
            [this, &csMsg](AddressList& addresses) -> AddressList {
            AddressList busyReceivers;
            for(const auto& addr : addresses)
            {
                auto errCode = sendMessage(csMsg, addr);
                if(errCode == ActionCallStatus::Success)
                {
                    //Logger::info("Sent message id: " ,  msg->operationID() ,  " from server side!");
                }
                else if(errCode == ActionCallStatus::ReceiverBusy)
                {
                    busyReceivers.emplace_back(std::move(addr));
                }
                else
                {
                    this->removeRegistersOfAddress(addr);
                    Logger::warn("Failed to send message id [" ,
                                 csMsg->operationID() ,
                                 "] to client " ,
                                 addr.dump()
                                 );
                }
            }
            return busyReceivers;
        };

        auto busyReceivers = trySendToDestinations(addresses);
        if(!busyReceivers.empty())
        {
            //If someones are busy, try with them once
            Logger::warn("Trying to send message to busy addresses once again!");
            busyReceivers = trySendToDestinations(busyReceivers);
        }

        //success when succeeded to send msg to at least one receiver
        success = (busyReceivers.size() != addresses.size());

    }
    return success ?
                   ActionCallStatus::ReceiverUnavailable
                   :
                ActionCallStatus::Success;
}

CSMsgContentBasePtr ServiceProviderImpl::getStatus(const OpID&  propertyID)
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

void ServiceProviderImpl::startServing()
{
    if(auto server = _server.lock())
    {
        server->registerServiceProvider(_delegator->shared_from_this());
    }
}

void ServiceProviderImpl::stopServing()
{
    if(auto server = _server.lock())
    {
        server->unregisterServiceProvider(_delegator->serviceID());
    }
}

ActionCallStatus ServiceProviderImpl::sendMessage(const CSMessagePtr &csMsg, const Address& toAddr)
{
    if(auto server = _server.lock())
    {
        return server->sendMessageToClient(csMsg, toAddr);
    }
    else
    {
        return ActionCallStatus::ReceiverUnavailable;
    }
}

ActionCallStatus ServiceProviderImpl::sendBackMessageToClient(const CSMessagePtr &csMsg)
{
    return sendMessage(csMsg, csMsg->sourceAddress());
}

void ServiceProviderImpl::onStatusChangeRegister(const CSMessagePtr &msg)
{
    // Do this for notifying status when changed
    saveRegisterInfo(msg);
    // Do this for server to update latest status for new registered client
    updateLatestStatus(msg);
}

void ServiceProviderImpl::onStatusChangeUnregister(const CSMessagePtr &msg)
{
    auto request = pickOutRequestInfo(msg);
    if(request)
    {
        request->invalidate();
    }

    removeRegisterInfo(msg);
}


ServiceProviderImpl::RequestPtr ServiceProviderImpl::saveRequestInfo(const CSMessagePtr &msg)
{
    RequestPtr request{ new Request(msg, _delegator->weak_from_this()) };
    (*_requestsMap.atomic())[msg->operationID()].push_back(request);
    return request;
}

ServiceProviderImpl::RequestPtr ServiceProviderImpl::pickOutRequestInfo(const CSMessagePtr &msg)
{
    RequestPtr request;
    std::lock_guard lock(_requestsMap);
    auto itRequestList = _requestsMap->find(msg->operationID());
    if(itRequestList != _requestsMap->end())
    {
        auto& requestList = itRequestList->second;
        for(auto itRequest = requestList.begin(); itRequest != requestList.end(); ++itRequest)
        {
            RequestPtr&  requestTmp = *itRequest;
            if(requestTmp->getRequestID() == msg->requestID())
            {
                request = std::move(requestTmp);
                requestList.erase(itRequest);
                break;
            }
        }
    }
    return request;
}

void ServiceProviderImpl::invalidateAndRemoveAllRequests()
{
    std::lock_guard lock(_requestsMap);
    for(auto& [opID, requests] : *_requestsMap)
    {
        for(auto& request : requests)
        {
            request->invalidate();
            if (auto abortCallback = request->getAbortCallback())
            {
                abortCallback();
            }
        }
    }
    _regEntriesMap->clear();
}

void ServiceProviderImpl::saveRegisterInfo(const CSMessagePtr &msg)
{
    (*_regEntriesMap.atomic())[msg->sourceAddress()].insert(msg->operationID());
}

void ServiceProviderImpl::removeRegisterInfo(const CSMessagePtr &msg)
{
    (*_regEntriesMap.atomic())[msg->sourceAddress()].erase(msg->operationID());
}

void ServiceProviderImpl::removeAllRegisterInfo()
{
    _regEntriesMap.atomic()->clear();
}

void ServiceProviderImpl::removeRegistersOfAddress(const Address &addr)
{
    _regEntriesMap.atomic()->erase(addr);
}

void ServiceProviderImpl::onAbortActionRequest(const CSMessagePtr &msg)
{
    if(auto request = pickOutRequestInfo(msg))
    {
        // Invalidate request then later respond from request itself will not cause any race condition.
        // Must be considered carefully for bug fixing later
        request->invalidate();
        if (auto abortCallback = request->getAbortCallback())
        {
            abortCallback();
        }
    }
}

void ServiceProviderImpl::onActionRequest(const CSMessagePtr &msg)
{
    auto request = saveRequestInfo(msg);
    if(!invokeRequestHandlerCallback(request))
    {
        //TODO: must provide error code to client?
        request->respond({});

        Logger::error("Not found handler for ActionRequest with OpID[",
                      msg->operationID(),
                      "]");
    }
}

void ServiceProviderImpl::onClientGoesOff(const CSMessagePtr &msg)
{
    _regEntriesMap.atomic()->erase(msg->sourceAddress());
}

bool ServiceProviderImpl::registerRequestHandler(
    const OpID&  opID,
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
            "Trying to set empty function as handler for OpID[",
            opID,
            "]"
            );
    }
    return false;
}

bool ServiceProviderImpl::unregisterRequestHandler(const OpID&  opID)
{
    return _requestHandlerMap.atomic()->erase(opID) != 0;
}

void ServiceProviderImpl::updateLatestStatus(const CSMessagePtr &registerMsg)
{
    auto currentStatus = getStatus(registerMsg->operationID());
    if(!currentStatus)
    {
        logging::Logger::warn(
            "status data of OpID[",
            registerMsg->operationID(),
            "] hasn't been set yet!"
            );
    }
    registerMsg->setContent(currentStatus);
    respondToRequest(registerMsg);
}

void ServiceProviderImpl::onStatusGetRequest(const CSMessagePtr& getMsg)
{
    auto request = saveRequestInfo(getMsg);
    if(!invokeRequestHandlerCallback(request))
    {
        request->respond(
            getStatus(request->getOperationID())
            );
    }
}

bool ServiceProviderImpl::invokeRequestHandlerCallback(
    const RequestPtr& request
    )
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

    return false;
}

}
}
