#include "thaf/Messaging/IPC/IPCServiceStub.h"
#include "thaf/Utils/Debugging/Debug.h"

namespace thaf {
namespace messaging {
namespace ipc {


void IPCServiceStub::onIPCMessage(const IPCMsgPtr &msg)
{
    thafInfo("Received Incoming Message: " <<
             "\nCode: " << msg->get_operation_code() <<
             "\nID: " << msg->get_operation_id() <<
             "\nSenderAddress: " << msg->get_sender_addr().dump());

    switch (msg->get_operation_code())
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
        thafWarn("Unhandled OpCode: " << msg->get_operation_code());
        break;
    }
}

IPCServiceStub::IPCServiceStub(IPCServiceStubHandler *stubHandler) :
    IPCCommunicator (),
    _stubHandler(stubHandler)
{
}

IPCServiceStub::~IPCServiceStub()
{
    deinit();
}

void IPCServiceStub::init(IPCType ipcType, Address myAddr)
{
    IPCCommunicator::init(ipcType, std::move(myAddr));
}

void IPCServiceStub::deinit()
{
    IPCCommunicator::deinit();
    removeAllRegisterInfo();
    invalidateAndRemoveAllRequestTrackers();
}

bool IPCServiceStub::replyToRequest(const IPCServiceStub::IPCMsgPtr &msg)
{
    auto errCode = ConnectionErrorCode::Failed;
    auto requestTracker = removeRequestInfo(msg);
    if(requestTracker)
    {
        requestTracker->invalidate();
        errCode = send(msg, msg->get_sender_addr());
    }
    return errCode == ConnectionErrorCode::Success;
}

bool IPCServiceStub::sendStatusUpdate(const IPCMsgPtr &ipcMsg)
{
    using AddressList = std::vector<Address>;
    bool success = false;
    AddressList addresses;

    { // locking _regEntriesMap block
        auto lock = _regEntriesMap.a_lock();
        for(auto& regEntry : *_regEntriesMap)
        {
            if(regEntry.second.find(ipcMsg->get_operation_id()) != regEntry.second.end())
            {
                auto& clientAddress = regEntry.first;
                addresses.push_back(clientAddress);
            }
        }
    } // locking _regEntriesMap block

    if(addresses.empty())
    {
        thafWarn("There's no register for property: " << ipcMsg->get_operation_id());
    }
    else
    {
        auto trySendToDestinations = [this, &ipcMsg](AddressList& addresses) -> AddressList {
            AddressList busyReceivers;
            for(const auto& addr : addresses)
            {
                auto errCode = send(ipcMsg, addr);
                if(errCode == ConnectionErrorCode::Success)
                {
                    thafInfo("Sent message id: " << ipcMsg->get_operation_id() << " from server side!");
                }
                else if(errCode == ConnectionErrorCode::Busy)
                {
                    busyReceivers.emplace_back(std::move(addr));
                }
                else
                {
                    this->removeRegistersOfAddress(addr);
                    thafWarn("Failed to send message id [" << ipcMsg->get_operation_id() << "] to client " << addr.dump());
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


void IPCServiceStub::onStatusChangeRegister(const IPCMsgPtr &msg)
{
    if(msg)
    {
        if(msg->get_sender_addr() != Address::INVALID_ADDRESS) //TBD: Use MessageValidator???
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
            thafErr("The client side did not provide address " << msg->get_sender_addr().dump());
        }
    }
}

void IPCServiceStub::onStatusChangeUnregister(const IPCMsgPtr &msg)
{
    auto requestTracker = removeRequestInfo(msg);
    if(requestTracker)
    {
        requestTracker->invalidate();
    }

    removeRegisterInfo(msg);
}


void IPCServiceStub::forwardToStubHandler(const IPCServiceStub::IPCRequestTrackerPtr &clp)
{
    if (_stubHandler)
    {
        _stubHandler->onClientRequest(clp);
    }
    else
    {
        thafWarn("hasn't set stub handler for this service stub yet!");
    }
}

IPCServiceStub::IPCRequestTrackerPtr IPCServiceStub::saveRequestInfo(const IPCServiceStub::IPCMsgPtr &msg)
{
    auto RequestTracker = IPCRequestTracker::create(msg, this);
    auto lock = _requestTrackersMap.a_lock();

    (*_requestTrackersMap)[msg->get_operation_id()].push_back(RequestTracker);

    return RequestTracker;
}

IPCServiceStub::IPCRequestTrackerPtr IPCServiceStub::removeRequestInfo(const IPCMsgPtr &msg)
{
    IPCRequestTrackerPtr clp;
    auto lock = _requestTrackersMap.a_lock();
    auto itListOfClps = _requestTrackersMap->find(msg->get_operation_id());
    if(itListOfClps != _requestTrackersMap->end())
    {
        auto& listOfClps = itListOfClps->second;
        for(auto itClp = listOfClps.begin(); itClp != listOfClps.end(); ++itClp)
        {
            IPCRequestTrackerPtr&  clpTmp = *itClp;
            if(clpTmp->valid() && clpTmp->_ipcMsg->get_request_id() == msg->get_request_id())
            {
                clp = clpTmp;
                listOfClps.erase(itClp);
                break;
            }
        }
    }

    return clp;
}

void IPCServiceStub::invalidateAndRemoveAllRequestTrackers()
{
    auto reqTrackerLock = _requestTrackersMap.a_lock();
    for(auto& reqTrackersPair : *_requestTrackersMap)
    {
        auto& requestTrackers = reqTrackersPair.second;
        for(auto& tracker : requestTrackers)
        {
            tracker->invalidate();
        }
    }
    _regEntriesMap->clear();
}

void IPCServiceStub::saveRegisterInfo(const IPCServiceStub::IPCMsgPtr &msg)
{
    auto lock = _regEntriesMap.a_lock();
    auto& interestedPropsOfThisSender = (*_regEntriesMap)[msg->get_sender_addr()];
    interestedPropsOfThisSender.insert(msg->get_operation_id());
}

void IPCServiceStub::removeRegisterInfo(const IPCServiceStub::IPCMsgPtr &msg)
{
    auto lock = _regEntriesMap.a_lock();
    auto& interestedPropsOfThisSender = (*_regEntriesMap)[msg->get_sender_addr()];
    interestedPropsOfThisSender.erase(msg->get_operation_id());
}

void IPCServiceStub::removeAllRegisterInfo()
{
    auto lock = _regEntriesMap.a_lock();
    _regEntriesMap->clear();
}

void IPCServiceStub::removeRegistersOfAddress(const Address &addr)
{
    auto lock = _regEntriesMap.a_lock();
    _regEntriesMap->erase(addr);
}

void IPCServiceStub::onAbortActionRequest(const IPCServiceStub::IPCMsgPtr &msg)
{
    IPCRequestTrackerPtr clp = removeRequestInfo(msg);
    if(clp)
    {
        clp->invalidate();
        forwardToStubHandler(clp);
    }
}

void IPCServiceStub::onClientRequest(const IPCServiceStub::IPCMsgPtr &msg)
{
    auto clp = saveRequestInfo(msg);
    forwardToStubHandler(clp);
}

void IPCServiceStub::setStubHandler(IPCServiceStubHandler *stubHandler)
{
    _stubHandler = stubHandler;
}


} // ipc
} // messaging
} // thaf
