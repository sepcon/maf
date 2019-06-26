#include "thaf/Messaging/IPC/IPCServiceProxy.h"
#include "thaf/Utils/Debugging/Debug.h"

namespace thaf {
namespace messaging {
namespace ipc {

void IPCServiceProxy::onIPCMessage(const std::shared_ptr<IPCMessage> &msg)
{
    if(msg)
    {
        switch (msg->get_operation_code()) {
        case OpCode::Register:
            onPropChangeUpdate(msg);
            break;
        case OpCode::Request:
            onRequestResult(msg);
            break;
        case OpCode::RequestSync:
            onRequestSyncResult(msg);
            break;
        default:
            thafErr("Invalid opcode");
            break;
        }
    }
}

void IPCServiceProxy::init(IPCType ipcType, Address receiverAddr)
{
    _stopFlag.store(false, std::memory_order_release);
    IPCCommunicator::init(ipcType, std::move(receiverAddr), true);
}

void IPCServiceProxy::deinit()
{
    _stopFlag.store(true, std::memory_order_release);
    IPCCommunicator::deinit();
    abortAllSyncRequest();
    if(_serviceMonitoringThread.joinable())
    {
        _serviceMonitoringThread.join();
    }
}

IPCServiceProxy::~IPCServiceProxy()
{
    deinit();
}

RegID IPCServiceProxy::sendRequest(
    const std::shared_ptr<IPCMessage>& outgoingMsg,
    IPCServiceProxy::MessageHandlerCallback callback
    )
{
    return storeAndSendRequestToServer(_requestEntriesMap, outgoingMsg, callback);
}

void IPCServiceProxy::sendAbortRequest(const RegID &regID)
{
    if(regID.opID != OpIDInvalid)
    {
        bool found = false;
        { // create {block} for releasing lock on _requestEntriesMap
            auto lk = _requestEntriesMap.a_lock();
            auto it = _requestEntriesMap->find(regID.opID);
            if(it != _requestEntriesMap->end())
            {
                auto& listOfRequests = it->second;
                for(auto requestEntryIt = listOfRequests.begin(); requestEntryIt != listOfRequests.end(); ++requestEntryIt)
                {
                    if(requestEntryIt->requestID == regID.requestID)
                    {
                        listOfRequests.erase(requestEntryIt);
                        found = true;
                        break;
                    }
                }
            }
        }

        if(found)
        {
            send(createIPCMsg(OpCode::Abort, regID.opID, srz::ByteArray{}, Address{}, regID.requestID));
            RegID::reclaimID(regID, _idMgr);
        }
    }
}

void IPCServiceProxy::sendAbortSyncRequest(const RegID &regID)
{
    removeSyncRegEntry(regID);
    send(createIPCMsg(OpCode::Abort, regID.opID));
}

bool IPCServiceProxy::sendRequestSync(const std::shared_ptr<IPCMessage> &outgoingMsg, IPCServiceProxy::MessageHandlerCallback callback, unsigned long maxWaitTimeMs)
{
    RegID regID;
    bool success = false;
    if(!_stopFlag.load(std::memory_order_acquire))
    {
        auto resultFuture = storeSyncRegEntry(outgoingMsg, regID);
        if(resultFuture && sendMessageToServer(outgoingMsg))
        {
            try
            {
                if(resultFuture->wait_for(std::chrono::milliseconds(maxWaitTimeMs)) == std::future_status::ready)
                {
                    if(auto msg = resultFuture->get())
                    {
                        callback(msg);
                        success = true;
                    }
                    else
                    {
                        thafWarn("Request id: " << regID.requestID << " has been cancelled");
                    }
                }
                else
                {
                    sendAbortSyncRequest(regID);
                }
            }
            catch(const std::exception& e)
            {
                thafErr("Error while waiting for result from server(Exception): " << e.what());
            }
            catch(...)
            {
                thafErr("Unknown exception when sending sync request to server");
            }
        }
    }
    else
    {
        thafErr("Could not send message after stopped!");
    }
    return success;
}

void IPCServiceProxy::startMonitoringService(ServiceStatusObserver *observer, long checkingPeriodMs)
{
    _serviceMonitoringThread = std::thread {[this, observer, checkingPeriodMs]{
        static thread_local ConnectionStatus currentStatus = ConnectionStatus::UnAvailable;
        while(!_stopFlag.load(std::memory_order_acquire))
        {
            auto status = _pSender->checkServerStatus();
            if(status != currentStatus)
            {
                if(status == ConnectionStatus::UnAvailable)
                {
                    thafWarn("Server has not been available for " << checkingPeriodMs << " second");
                }
                observer->onStatusChanged(currentStatus, status);
                currentStatus = status;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(checkingPeriodMs));
        }
    }};
}

RegID IPCServiceProxy::sendStatusChangeRegister(OpID propertyID, IPCServiceProxy::MessageHandlerCallback callback)
{
    return storeAndSendRequestToServer(_registerEntriesMap, createIPCMsg(OpCode::Register, propertyID), callback);
}

void IPCServiceProxy::sendStatusChangeUnregister(RegID regID)
{
    if(regID.valid())
    {
        auto propertyID = regID.opID;
        auto totalRemainder = removeRegEntry(_registerEntriesMap, regID);
        if(totalRemainder == 0)
        {
            //send unregister if no one from client side interested in this propertyID anymore
            send(createIPCMsg(OpCode::UnRegister, propertyID));
        }
    }
    else
    {
        thafWarn("Try to Unregister invalid RegID");
    }
}

void IPCServiceProxy::onPropChangeUpdate(const std::shared_ptr<IPCMessage>& msg)
{
    std::vector<decltype(RegEntry::callback)> callbacks;

    {
        auto lk = _registerEntriesMap.a_lock();
        auto it = _registerEntriesMap->find(msg->get_operation_id());
        if(it != _registerEntriesMap->end())
        {
            for(auto& regEntry : it->second)
            {
                callbacks.push_back(regEntry.callback);
            }
        }
    }

    for(auto& callback : callbacks)
    {
        callback(msg);
    }
}

void IPCServiceProxy::onRequestResult(const std::shared_ptr<IPCMessage>& msg)
{
    decltype (RegEntry::callback) callback;
    {
        auto lk = _requestEntriesMap.a_lock();
        auto it = _requestEntriesMap->find(msg->get_operation_id());
        if(it != _requestEntriesMap->end())
        {
            auto& regEntries = it->second;
            auto itRegEntry = regEntries.begin();
            for(; itRegEntry != regEntries.end(); ++itRegEntry)
            {
                if(itRegEntry->requestID == msg->get_request_id())
                {
                    break;
                }
            }
            callback = std::move(itRegEntry->callback);
            regEntries.erase(itRegEntry);
        }
    }

    callback(msg);
}

void IPCServiceProxy::onRequestSyncResult(const std::shared_ptr<IPCMessage> &msg)
{
    if(auto resultPromise = removeSyncRegEntry(RegID{ msg->get_request_id(), msg->get_operation_id() }))
    {
        resultPromise->set_value(msg);
    }
    else
    {
        thafWarn("Could not find RegEntry for operationID [" << msg->get_operation_id() << "] and requestID [" << msg->get_request_id() << "]");
    }
}

void IPCServiceProxy::abortAllSyncRequest()
{
    int totalAborted = 0;
    auto lock = _syncRequestEntriesMap.a_lock();
    for(auto& opid2ListEntries : *_syncRequestEntriesMap)
    {
        for(auto& regEntry : opid2ListEntries.second)
        {
            ++ totalAborted;
            regEntry._msgPromise.set_value(nullptr);
        }
    }
    if(totalAborted > 0)
    {
        thafWarn("Aborting " << totalAborted << " Sync requests!");
    }
    _syncRequestEntriesMap->clear();
}

bool IPCServiceProxy::sendMessageToServer(const std::shared_ptr<IPCMessage> &outgoingMsg)
{
    ConnectionErrorCode errCode = ConnectionErrorCode::Failed;
    if(outgoingMsg->get_sender_addr() == Address::INVALID_ADDRESS)
    {
        outgoingMsg->set_sender_addr(_pReceiver->address());
    }

    //TBD: handle for case of not be able to send message
    //might be server is disconnected, or might server is being busy
    if((errCode = send(outgoingMsg)) != ConnectionErrorCode::Success)
    {
        thafErr("Failed to send message to server: " << _pSender->receiverAddress().dump());
    }
    return errCode == ConnectionErrorCode::Success;
}

std::shared_ptr<std::future<std::shared_ptr<IPCMessage> > > IPCServiceProxy::storeSyncRegEntry(const std::shared_ptr<IPCMessage> &outgoingMsg, RegID &regID)
{
    regID.requestID = _idMgr.allocateNewID();
    regID.opID = outgoingMsg->get_operation_id();

    std::shared_ptr<std::future<std::shared_ptr<IPCMessage> > > resultFuture;

    { // using {block} to unlock the mutex of _syncRequestEntriesMap
        auto lk = _syncRequestEntriesMap.a_lock();
        auto& listOfRequests = (*_syncRequestEntriesMap)[outgoingMsg->get_operation_id()];
        std::promise<std::shared_ptr<IPCMessage>> prm;
        resultFuture = std::make_shared<std::future<std::shared_ptr<IPCMessage> >>(prm.get_future());
        SyncRegEntry regEntry = { regID.requestID, std::move(prm) };
        listOfRequests.push_back(std::move(regEntry));
    }
    outgoingMsg->set_request_id(regID.requestID);
    return resultFuture;
}

std::shared_ptr<std::promise<std::shared_ptr<IPCMessage> >> IPCServiceProxy::removeSyncRegEntry(const RegID& regID)
{
    std::shared_ptr<std::promise<std::shared_ptr<IPCMessage> >> resultPromise;
    { // using {block} to unlock the mutex of _syncRequestEntriesMap
        auto lk = _syncRequestEntriesMap.a_lock();
        auto itPairID2ListOfRequests = _syncRequestEntriesMap->find(regID.opID);
        if(itPairID2ListOfRequests != _syncRequestEntriesMap->end())
        {
            auto& listOfRequests = itPairID2ListOfRequests->second;
            for(auto itRequestEntry = listOfRequests.begin(); itRequestEntry != listOfRequests.end(); ++itRequestEntry)
            {
                if(itRequestEntry->requestID == regID.requestID)
                {
                    resultPromise = std::make_shared<std::promise<std::shared_ptr<IPCMessage> > >(std::move(itRequestEntry->_msgPromise));
                    listOfRequests.erase(itRequestEntry);
                    break;
                }
            }
        }
    }
    return resultPromise;
}

RegID IPCServiceProxy::storeAndSendRequestToServer
    (
        RegEntriesMap& regEntriesMap,
        const std::shared_ptr<IPCMessage>& outgoingMsg,
        IPCServiceProxy::MessageHandlerCallback callback
        )
{
    RegID regID;
    auto totalSameRequests = storeRegEntry(regEntriesMap, outgoingMsg->get_operation_id(), callback, regID);
    // we have Proxy to reduce the number of requests to server, then in case of Register, if many classes call to register to same property, then only one will be sent
    bool shouldSendToServer = (outgoingMsg->get_operation_code() == OpCode::Register && totalSameRequests == 1 ) ||
                              (totalSameRequests >= 1 && outgoingMsg->get_operation_code() != OpCode::Register);

    if(shouldSendToServer)
    {
        outgoingMsg->set_request_id(regID.requestID);
        if(!sendMessageToServer(outgoingMsg))
        {
            removeRegEntry(regEntriesMap, regID);
            thafErr("Could not send message to address: " << _pReceiver->address().dump());
        }
    }
    return regID;
}

size_t IPCServiceProxy::storeRegEntry
    (RegEntriesMap& regInfoEntries,
     OpID propertyID,
     MessageHandlerCallback callback,
     RegID &regID
     )
{
    regID.requestID = _idMgr.allocateNewID();
    regID.opID = propertyID;

    auto lk = regInfoEntries.a_lock();
    auto& regEntries = (*regInfoEntries)[propertyID];
    regEntries.emplace_back( regID.requestID, std::move(callback) );
    return regEntries.size(); //means that already sent register for this propertyID to service
}

size_t IPCServiceProxy::removeRegEntry(RegEntriesMap &regInfoEntries, RegID &regID)
{
    auto lk = regInfoEntries.a_lock();
    auto it = regInfoEntries->find(regID.opID);
    if(it != regInfoEntries->end())
    {
        it->second.erase
            (
                std::remove_if
                (
                    it->second.begin(),
                    it->second.end(),
                    [&regID](const RegEntry& regEntry) { return regEntry.requestID == regID.requestID; }
                ),
                it->second.end()
            );
        RegID::reclaimID(regID, this->_idMgr);
        return it->second.size();
    }
    else
    {
        return 0;
    }
}



} // ipc
} // messaging
} // thaf
