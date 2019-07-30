#include "thaf/messaging/client-server/ServiceStubBase.h"
#include "thaf/messaging/client-server/interfaces/CSStatus.h"
#include "thaf/utils/debugging/Debug.h"

namespace thaf {
namespace messaging {


bool ServiceStubBase::onIncomingMessage(const CSMessagePtr &msg)
{
    thafInfo("Received Incoming Message: " <<
             "\nCode: " << msg->operationCode() <<
             "\nID: " << msg->operationID() <<
             "\nSenderAddress: " << msg->sourceAddress().dump());
    bool handled = true;
    switch (msg->operationCode())
    {
    case OpCode::Register:
        onStatusChangeRegister(msg);
        break;
    case OpCode::UnRegister:
        onStatusChangeUnregister(msg);
        break;
    case OpCode::Get:
    case OpCode::Request:
    case OpCode::RequestSync:
    case OpCode::Abort:
        onClientRequest(msg);
        break;
    default:
        handled = false;
        thafWarn("Unhandled OpCode: " << msg->operationCode());
        break;
    }
    return handled;
}

ServiceStubBase::ServiceStubBase(ServiceID sid, ServerInterface *server, ServiceStubHandlerInterface *stubHandler) :
    _stubHandler(stubHandler),
    _server(server)
{
    assert(_server && "Server must not be null");
    setServiceID(sid);
}


ServiceStubBase::~ServiceStubBase()
{
    removeAllRegisterInfo();
    invalidateAndRemoveAllRequestKeepers();
}


bool ServiceStubBase::replyToRequest(const CSMessagePtr& msg)
{
    auto errCode = DataTransmissionErrorCode::ReceiverUnavailable;
    auto requestKeeper = removeRequestInfo(msg);
    if(requestKeeper)
    {
        requestKeeper->invalidate();
        errCode = _server->sendMessageToClient(msg);
    }
    return errCode == DataTransmissionErrorCode::Success;
}

bool ServiceStubBase::sendStatusUpdate(const CSMessagePtr &msg)
{
    using AddressList = std::vector<Address>;
    bool success = false;
    AddressList addresses;

    { // locking _regEntriesMap block
        auto lock = _regEntriesMap.a_lock();
        for(auto& regEntry : *_regEntriesMap)
        {
            if(regEntry.second.find(msg->operationID()) != regEntry.second.end())
            {
                auto& clientAddress = regEntry.first;
                addresses.push_back(clientAddress);
            }
        }
    } // locking _regEntriesMap block

    if(addresses.empty())
    {
        thafWarn("There's no register for property: " << msg->operationID());
    }
    else
    {
        auto trySendToDestinations = [this, &msg](AddressList& addresses) -> AddressList {
            AddressList busyReceivers;
            for(const auto& addr : addresses)
            {
                auto errCode = _server->sendMessageToClient(msg, addr);
                if(errCode == DataTransmissionErrorCode::Success)
                {
                    thafInfo("Sent message id: " << msg->operationID() << " from server side!");
                }
                else if(errCode == DataTransmissionErrorCode::ReceiverBusy)
                {
                    busyReceivers.emplace_back(std::move(addr));
                }
                else
                {
                    this->removeRegistersOfAddress(addr);
                    thafWarn("Failed to send message id [" << msg->operationID() << "] to client " << addr.dump());
                }
            }
            return busyReceivers;
        };

        auto busyReceivers = trySendToDestinations(addresses);
        if(!busyReceivers.empty())
        {
            //If someones are busy, try with them once
            thafWarn("Trying to send message to busy addresses once again!");
            busyReceivers = trySendToDestinations(busyReceivers);
        }

        success = (busyReceivers.size() != addresses.size()); //success when success to send msg to at least one receiver

    }
    return success;
}


void ServiceStubBase::onStatusChangeRegister(const CSMessagePtr &msg)
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
            thafErr("The client side did not provide address " << msg->sourceAddress().dump());
        }
    }
}

void ServiceStubBase::onStatusChangeUnregister(const CSMessagePtr &msg)
{
    auto requestKeeper = removeRequestInfo(msg);
    if(requestKeeper)
    {
        requestKeeper->invalidate();
    }

    removeRegisterInfo(msg);
}


void ServiceStubBase::forwardToStubHandler(const ServiceStubBase::RequestKeeperPtr& requestKeeper)
{
    if (_stubHandler)
    {
        _stubHandler->onClientRequest(requestKeeper);
    }
    else
    {
        thafWarn("hasn't set stub handler for this service stub yet!");
    }
}

ServiceStubBase::RequestKeeperPtr ServiceStubBase::saveRequestInfo(const CSMessagePtr &msg)
{
    auto requestKeeper = RequestKeeperBase::create(msg, this);
    auto lock = _requestKeepersMap.a_lock();

    (*_requestKeepersMap)[msg->operationID()].push_back(requestKeeper);

    return requestKeeper;
}

ServiceStubBase::RequestKeeperPtr ServiceStubBase::removeRequestInfo(const CSMessagePtr &msg)
{
    RequestKeeperPtr keeper;
    auto lock = _requestKeepersMap.a_lock();
    auto itListOfClps = _requestKeepersMap->find(msg->operationID());
    if(itListOfClps != _requestKeepersMap->end())
    {
        auto& listOfClps = itListOfClps->second;
        for(auto itClp = listOfClps.begin(); itClp != listOfClps.end(); ++itClp)
        {
            RequestKeeperPtr&  keeperTmp = *itClp;
            if(keeperTmp->valid() && keeperTmp->_csMsg->requestID() == msg->requestID())
            {
                keeper = keeperTmp;
                listOfClps.erase(itClp);
                break;
            }
        }
    }

    return keeper;
}

void ServiceStubBase::invalidateAndRemoveAllRequestKeepers()
{
    auto reqTrackerLock = _requestKeepersMap.a_lock();
    for(auto& reqTrackersPair : *_requestKeepersMap)
    {
        auto& requestKeepers = reqTrackersPair.second;
        for(auto& tracker : requestKeepers)
        {
            tracker->invalidate();
        }
    }
    _regEntriesMap->clear();
}

void ServiceStubBase::saveRegisterInfo(const CSMessagePtr &msg)
{
    auto lock = _regEntriesMap.a_lock();
    auto& interestedPropsOfThisSender = (*_regEntriesMap)[msg->sourceAddress()];
    interestedPropsOfThisSender.insert(msg->operationID());
}

void ServiceStubBase::removeRegisterInfo(const CSMessagePtr &msg)
{
    auto lock = _regEntriesMap.a_lock();
    auto& interestedPropsOfThisSender = (*_regEntriesMap)[msg->sourceAddress()];
    interestedPropsOfThisSender.erase(msg->operationID());
}

void ServiceStubBase::removeAllRegisterInfo()
{
    auto lock = _regEntriesMap.a_lock();
    _regEntriesMap->clear();
}

void ServiceStubBase::removeRegistersOfAddress(const Address &addr)
{
    auto lock = _regEntriesMap.a_lock();
    _regEntriesMap->erase(addr);
}

void ServiceStubBase::onAbortActionRequest(const CSMessagePtr &msg)
{
    RequestKeeperPtr clp = removeRequestInfo(msg);
    if(clp)
    {
        clp->invalidate();
        forwardToStubHandler(clp);
    }
}

void ServiceStubBase::onClientRequest(const CSMessagePtr &msg)
{
    auto clp = saveRequestInfo(msg);
    forwardToStubHandler(clp);
}

void ServiceStubBase::setStubHandler(ServiceStubHandlerInterface *stubHandler)
{
    _stubHandler = stubHandler;
}


} // messaging
} // thaf
