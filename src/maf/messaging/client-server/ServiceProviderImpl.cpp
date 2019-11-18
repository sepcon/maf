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
    case OpCode::UnRegister:
        onStatusChangeUnregister(msg);
        break;
    case OpCode::UnregisterServiceStatus:
        onClientGoesOff(msg);
        break;
    case OpCode::Request:
    case OpCode::StatusGet:
        onClientRequest(msg);
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
    std::weak_ptr<ServerInterface> server,
    ServiceStubHandlerInterface *stubHandler) :
    _server(std::move(server)),
    _holder(std::move(holder)),
    _stubHandler(stubHandler),
    _stopped(false)
{
    assert(_server.lock() && "Server must not be null");
}


ServiceProviderImpl::~ServiceProviderImpl()
{
    if(auto server = _server.lock()) { server->unregisterServiceProvider(_holder->serviceID()); }
    removeAllRegisterInfo();
    invalidateAndRemoveAllRequests();
    _stopped.store(true, std::memory_order_release);
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

ActionCallStatus ServiceProviderImpl::setStatus(OpID propertyID, const CSMsgContentBasePtr &property)
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
        auto csMsg = createCSMessage(_holder->serviceID(), propertyID, OpCode::Register, RequestIDInvalid, property);
        auto trySendToDestinations = [this, &csMsg](AddressList& addresses) -> AddressList {
            AddressList busyReceivers;
            for(const auto& addr : addresses)
            {
                auto errCode = sendMessage(csMsg, addr);
                if(errCode == ActionCallStatus::Success)
                {
//                    Logger::info("Sent message id: " ,  msg->operationID() ,  " from server side!");
                }
                else if(errCode == ActionCallStatus::ReceiverBusy)
                {
                    busyReceivers.emplace_back(std::move(addr));
                }
                else
                {
                    this->removeRegistersOfAddress(addr);
                    Logger::warn("Failed to send message id [" ,  csMsg->operationID() ,  "] to client " ,  addr.dump());
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

        success = (busyReceivers.size() != addresses.size()); //success when success to send msg to at least one receiver

    }
    return success ? ActionCallStatus::ReceiverUnavailable : ActionCallStatus::Success;
}

void ServiceProviderImpl::startServing()
{
    if(auto server = _server.lock())
    {
        server->registerServiceProvider(_holder->shared_from_this());
    }
}

void ServiceProviderImpl::stopServing()
{
    if(auto server = _server.lock())
    {
        server->unregisterServiceProvider(_holder->serviceID());
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
    if(msg)
    {
        if(msg->sourceAddress() != Address::INVALID_ADDRESS) //TBD: Use MessageValidator???
        {
            // Do this for notifying status when changed
            saveRegisterInfo(msg);
            // Do this for server to update latest status for new registered client
            forwardToStubHandler(
                saveRequestInfo(msg)
                );
        }
        else
        {
            Logger::error("The client side did not provide address " ,  msg->sourceAddress().dump());
        }
    }
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


void ServiceProviderImpl::forwardToStubHandler(const ServiceProviderImpl::RequestPtr& request)
{
    if (_stubHandler)
    {
        _stubHandler->onClientRequest(request);
    }
    else
    {
        Logger::warn("hasn't set stub handler for this service stub yet!");
    }
}

void ServiceProviderImpl::forwardToStubHandler(RequestAbortedCallback callback)
{
    if (_stubHandler)
    {
        _stubHandler->onClientAbortRequest(std::move(callback));
    }
    else
    {
        Logger::warn("hasn't set stub handler for this service stub yet!");
    }
}

ServiceProviderImpl::RequestPtr ServiceProviderImpl::saveRequestInfo(const CSMessagePtr &msg)
{
    RequestPtr request{ new Request(msg, _holder->weak_from_this()) };
    (*_requestsMap.atomic())[msg->operationID()].push_back(request);
    return request;
}

ServiceProviderImpl::RequestPtr ServiceProviderImpl::pickOutRequestInfo(const CSMessagePtr &msg)
{
    RequestPtr request;
    if(!_stopped.load(std::memory_order_acquire))
    {
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
            auto abortCallback = request->getAbortCallback();
            if (abortCallback && _stubHandler)
            {
                forwardToStubHandler(std::move(abortCallback));
            }
            else
            {
                request->setOperationCode(OpCode::Abort);
                forwardToStubHandler(request);
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
        auto abortCallback = request->getAbortCallback();
        if (abortCallback && _stubHandler)
        {
            forwardToStubHandler(std::move(abortCallback));
        }
        else
        {
            request->_csMsg = msg;
            forwardToStubHandler(request);
        }
    }
}

void ServiceProviderImpl::onClientRequest(const CSMessagePtr &msg)
{
    forwardToStubHandler(saveRequestInfo(msg));
}

void ServiceProviderImpl::onClientGoesOff(const CSMessagePtr &msg)
{
    _regEntriesMap.atomic()->erase(msg->sourceAddress());
}

void ServiceProviderImpl::setStubHandler(ServiceStubHandlerInterface *stubHandler)
{
    if(!_stubHandler)
    {
        _stubHandler = stubHandler;
    }
    else if(stubHandler)
    {
        Logger::warn("Current stub handler of service id ", _holder->serviceID(), " got replaced by a different one!");
    }
    else
    {
        Logger::info("Unregistered stub handler for service id ", _holder->serviceID());
    }
}

}
}
