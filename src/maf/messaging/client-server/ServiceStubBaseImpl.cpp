#include "ServiceStubBaseImpl.h"
#include <maf/messaging/client-server/ServiceStubHandlerInterface.h>
#include <vector>


namespace maf {
namespace messaging {

void setResultUpdateCode(const CSMessagePtr& csMsg)
{
    OpCode rescode = csMsg->operationCode();
    switch (csMsg->operationCode()) {
    case OpCode::Request:
        rescode = OpCode::RequestResultUpdate;
        break;
    case OpCode::RequestSync:
        rescode = OpCode::RequestSyncResultUpdate;
        break;
    case OpCode::Register:
    case OpCode::UnRegister:
        rescode = OpCode::StatusUpdate;
        break;
    default:
        break;
    }
    csMsg->setOperationCode(rescode);
}
void setResultDoneCode(const CSMessagePtr& csMsg)
{
    OpCode rescode = csMsg->operationCode();
    switch (csMsg->operationCode()) {
    case OpCode::Request:
        rescode = OpCode::RequestResultDone;
        break;
    case OpCode::RequestSync:
        rescode = OpCode::RequestSyncResultDone;
        break;
    case OpCode::Register:
    case OpCode::UnRegister:
        rescode = OpCode::StatusUpdate;
        break;
    default:
        break;
    }
    csMsg->setOperationCode(rescode);
}

void applyResponseCode(const CSMessagePtr& csMsg, bool done = true)
{
    if(done)
    {
        setResultDoneCode(csMsg);
    }
    else
    {
        setResultUpdateCode(csMsg);
    }
}

bool ServiceStubBaseImpl::onIncomingMessage(const CSMessagePtr &msg)
{
    mafInfo("Received Incoming Message: " <<
            "\n\tCode: " << msg->operationCode() <<
            "\n\tID: " << msg->operationID() <<
            "\n\tSenderAddress: " << msg->sourceAddress().dump(2));
    bool handled = true;
    switch (msg->operationCode())
    {
    case OpCode::Register:
        onStatusChangeRegister(msg);
        break;
    case OpCode::UnRegister:
        onStatusChangeUnregister(msg);
        break;
    case OpCode::Request:
    case OpCode::RequestSync:
        onClientRequest(msg);
        break;
    case OpCode::Abort:
        onAbortActionRequest(msg);
        break;
    default:
        handled = false;
        mafWarn("Unhandled OpCode: " << msg->operationCode());
        break;
    }
    return handled;
}

ServiceStubBaseImpl::ServiceStubBaseImpl(ServerInterface *server, ServiceStubHandlerInterface *stubHandler) :
    _stubHandler(stubHandler),
    _server(server),
    _stopped(false)
{
    assert(_server && "Server must not be null");
}


ServiceStubBaseImpl::~ServiceStubBaseImpl()
{
    _stopped.store(true, std::memory_order_release);
    removeAllRegisterInfo();
    invalidateAndRemoveAllRequestKeepers();
}


bool ServiceStubBaseImpl::replyToRequest(const CSMessagePtr& csMsg, bool hasDone)
{
    //Sync request must always be done at first response from service. do not allow client to be block by many updates
    if(csMsg->operationCode() == OpCode::RequestSync) { hasDone = true; }
    auto requestKeeper = pickOutRequestInfo(csMsg, hasDone);
    if(requestKeeper)
    {
        if(hasDone)
        {
            requestKeeper->invalidateIfValid();
        }
        return feedbackToClient(csMsg, hasDone);
    }
    else
    {
        return false;
    }
}

bool ServiceStubBaseImpl::sendStatusUpdate(const CSMessagePtr &msg)
{
    using AddressList = std::vector<Address>;
    bool success = false;
    AddressList addresses;

    { // locking _regEntriesMap block
        std::lock_guard lock(_regEntriesMap);
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
        mafWarn("There's no register for property: " << msg->operationID());
    }
    else
    {
        applyResponseCode(msg);
        auto trySendToDestinations = [this, &msg](AddressList& addresses) -> AddressList {
            AddressList busyReceivers;
            for(const auto& addr : addresses)
            {
                auto errCode = _server->sendMessageToClient(msg, addr);
                if(errCode == DataTransmissionErrorCode::Success)
                {
                    //                    mafInfo("Sent message id: " << msg->operationID() << " from server side!");
                }
                else if(errCode == DataTransmissionErrorCode::ReceiverBusy)
                {
                    busyReceivers.emplace_back(std::move(addr));
                }
                else
                {
                    this->removeRegistersOfAddress(addr);
                    mafWarn("Failed to send message id [" << msg->operationID() << "] to client " << addr.dump());
                }
            }
            return busyReceivers;
        };

        auto busyReceivers = trySendToDestinations(addresses);
        if(!busyReceivers.empty())
        {
            //If someones are busy, try with them once
            mafWarn("Trying to send message to busy addresses once again!");
            busyReceivers = trySendToDestinations(busyReceivers);
        }

        success = (busyReceivers.size() != addresses.size()); //success when success to send msg to at least one receiver

    }
    return success;
}

bool ServiceStubBaseImpl::feedbackToClient(const CSMessagePtr &csMsg, bool hasDone)
{
    applyResponseCode(csMsg, hasDone);
    return _server->sendMessageToClient(csMsg, csMsg->sourceAddress()) == DataTransmissionErrorCode::Success;
}


void ServiceStubBaseImpl::onStatusChangeRegister(const CSMessagePtr &msg)
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
            mafErr("The client side did not provide address " << msg->sourceAddress().dump());
        }
    }
}

void ServiceStubBaseImpl::onStatusChangeUnregister(const CSMessagePtr &msg)
{
    auto requestKeeper = pickOutRequestInfo(msg);
    if(requestKeeper)
    {
        requestKeeper->invalidateIfValid();
    }

    removeRegisterInfo(msg);
}


void ServiceStubBaseImpl::forwardToStubHandler(const ServiceStubBaseImpl::RequestKeeperPtr& requestKeeper)
{
    if (_stubHandler)
    {
        _stubHandler->onClientRequest(requestKeeper);
    }
    else
    {
        mafWarn("hasn't set stub handler for this service stub yet!");
    }
}

void ServiceStubBaseImpl::forwardToStubHandler(RequestKeeperBase::AbortCallback callback)
{
    if (_stubHandler)
    {
        _stubHandler->onClientAbortRequest(std::move(callback));
    }
    else
    {
        mafWarn("hasn't set stub handler for this service stub yet!");
    }
}

ServiceStubBaseImpl::RequestKeeperPtr ServiceStubBaseImpl::saveRequestInfo(const CSMessagePtr &msg)
{
    auto requestKeeper = RequestKeeperBase::create(msg, this);

    (*_requestKeepersMap.atomic())[msg->operationID()].push_back(requestKeeper);

    return requestKeeper;
}

ServiceStubBaseImpl::RequestKeeperPtr ServiceStubBaseImpl::pickOutRequestInfo(const CSMessagePtr &msg, bool remove)
{
    RequestKeeperPtr keeper;
    // Using _stopped flag to prevent RequestKeeper from removing itself from the map while destructing ServiceStubBaseImpl object
    // By that deadlock situation will not occurr
    if(!_stopped.load(std::memory_order_acquire))
    {
        std::lock_guard lock(_requestKeepersMap);
        auto itKeeperList = _requestKeepersMap->find(msg->operationID());
        if(itKeeperList != _requestKeepersMap->end())
        {
            auto& listOfClps = itKeeperList->second;
            for(auto itClp = listOfClps.begin(); itClp != listOfClps.end(); ++itClp)
            {
                RequestKeeperPtr&  keeperTmp = *itClp;
                if(keeperTmp->_csMsg->requestID() == msg->requestID())
                {
                    keeper = keeperTmp;
                    if(remove)
                    {
                        listOfClps.erase(itClp);
                    }
                    break;
                }
            }
        }
    }
    return keeper;
}

void ServiceStubBaseImpl::invalidateAndRemoveAllRequestKeepers()
{
    std::lock_guard lock(_requestKeepersMap);
    for(auto& [opID, requestKeepers] : *_requestKeepersMap)
    {
        for(auto& keeper : requestKeepers)
        {
            keeper->invalidateIfValid();
        }
    }
    _regEntriesMap->clear();
}

void ServiceStubBaseImpl::saveRegisterInfo(const CSMessagePtr &msg)
{
    (*_regEntriesMap.atomic())[msg->sourceAddress()].insert(msg->operationID());
}

void ServiceStubBaseImpl::removeRegisterInfo(const CSMessagePtr &msg)
{
    (*_regEntriesMap.atomic())[msg->sourceAddress()].erase(msg->operationID());
}

void ServiceStubBaseImpl::removeAllRegisterInfo()
{
    _regEntriesMap.atomic()->clear();
}

void ServiceStubBaseImpl::removeRegistersOfAddress(const Address &addr)
{
    _regEntriesMap.atomic()->erase(addr);
}

void ServiceStubBaseImpl::onAbortActionRequest(const CSMessagePtr &msg)
{
    RequestKeeperPtr keeper = pickOutRequestInfo(msg);
    if(keeper && (keeper->invalidateIfValid()))
    {
        auto abortCallback = keeper->getAbortCallback();
        if (abortCallback && _stubHandler)
        {
            forwardToStubHandler(std::move(abortCallback));
        }
        else
        {
            keeper->_csMsg = msg;
            forwardToStubHandler(keeper);
        }
    }
}

void ServiceStubBaseImpl::onClientRequest(const CSMessagePtr &msg)
{
    auto keeper = saveRequestInfo(msg);
    forwardToStubHandler(keeper);
}

void ServiceStubBaseImpl::setStubHandler(ServiceStubHandlerInterface *stubHandler)
{
    _stubHandler = stubHandler;
}

}
}
