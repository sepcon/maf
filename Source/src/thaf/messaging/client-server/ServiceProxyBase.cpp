#include "thaf/messaging/client-server/ServiceProxyBase.h"
#include "thaf/messaging/client-server/interfaces/ClientInterface.h"
#include "thaf/utils/debugging/Debug.h"


namespace thaf {
namespace messaging {

bool ServiceProxyBase::onIncomingMessage(const CSMessagePtr &msg)
{
    bool handled = true;
    if(msg && msg->serviceID() == serviceID())
    {
        switch (msg->operationCode()) {
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
            handled = false;
            thafErr("Invalid opcode");
            break;
        }
    }
    return handled;
}


ServiceProxyBase::ServiceProxyBase(ServiceID sid, ClientInterface *client) : _client(client)
{
    setServiceID(sid);
}

ServiceProxyBase::~ServiceProxyBase()
{
    abortAllSyncRequest();
}

RegID ServiceProxyBase::sendRequest(const CSMsgContentPtr &msgContent,
    CSMessageHandlerCallback callback
    )
{
    RegID regID = {};
    auto csMsg = createCSMessage(msgContent->operationID(), OpCode::Request, msgContent);
    if(callback)
    {
        regID = storeAndSendRequestToServer(_requestEntriesMap, csMsg, callback);
    }
    else
    {
        _client->sendMessageToServer(csMsg);
    }
    return regID;
}

void ServiceProxyBase::sendAbortRequest(const RegID &regID)
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
            auto msg = createCSMessage(regID.opID, OpCode::Abort);
            msg->setRequestID(regID.requestID);
            _client->sendMessageToServer(msg);
            RegID::reclaimID(regID, _idMgr);
        }
    }
}

void ServiceProxyBase::sendAbortSyncRequest(const RegID &regID)
{
    removeSyncRegEntry(regID);
    _client->sendMessageToServer(createCSMessage(regID.opID, OpCode::Abort));
}

bool ServiceProxyBase::sendRequestSync(const CSMsgContentPtr &msgContent, CSMessageHandlerCallback callback, unsigned long maxWaitTimeMs)
{
    RegID regID;
    bool success = false;
    auto csMsg = createCSMessage(msgContent->operationID(), OpCode::RequestSync, msgContent);
    if(callback)
    {
        auto resultFuture = storeSyncRegEntry(csMsg, regID);
        if(resultFuture && sendMessageToServer(csMsg))
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
    return success;
}

void ServiceProxyBase::onServerStatusChanged(Availability oldStatus, Availability newStatus)
{
    thafInfo("Server Status changed from: " << static_cast<int>(oldStatus) << " to " << static_cast<int>(newStatus));
}

void ServiceProxyBase::onServiceStatusChanged(ServiceID sid, Availability oldStatus, Availability newStatus)
{
    thafInfo("ServiceID [" << sid << "] has changed status from: " << static_cast<int>(oldStatus) << " to " << static_cast<int>(newStatus));
}

CSMessagePtr ServiceProxyBase::createCSMessage(OpID opID, OpCode opCode, const CSMsgContentPtr &msgContent)
{
    return messaging::createCSMessage(serviceID(), std::move(opID), std::move(opCode), RequestIDInvalid, msgContent);
}


//void ServiceProxyBase::startMonitoringService(ServiceStatusObserver *observer, long checkingPeriodMs)
//{
//    _serviceMonitoringThread = std::thread {[this, observer, checkingPeriodMs]{
//        static thread_local Availability currentStatus = Availability::UnAvailable;
//        while(!_stopFlag.load(std::memory_order_acquire))
//        {
//            auto status = _client->checkReceiverStatus();
//            if(status != currentStatus)
//            {
//                if(status == Availability::UnAvailable)
//                {
//                    thafWarn("Server has not been available for " << checkingPeriodMs << " second");
//                }
//                observer->onStatusChanged(currentStatus, status);
//                currentStatus = status;
//            }
//            std::this_thread::sleep_for(std::chrono::milliseconds(checkingPeriodMs));
//        }
//    }};
//}

RegID ServiceProxyBase::sendStatusChangeRegister(OpID propertyID, CSMessageHandlerCallback callback)
{
    return storeAndSendRequestToServer(
        _registerEntriesMap,
        createCSMessage(propertyID, OpCode::Register),
        callback
        );
}

void ServiceProxyBase::sendStatusChangeUnregister(RegID regID)
{
    if(regID.valid())
    {
        auto propertyID = regID.opID;
        auto totalRemainder = removeRegEntry(_registerEntriesMap, regID);
        if(totalRemainder == 0)
        {
            //send unregister if no one from client side interested in this propertyID anymore
            _client->sendMessageToServer(createCSMessage(propertyID, OpCode::UnRegister));
        }
    }
    else
    {
        thafWarn("Try to Unregister invalid RegID");
    }
}

void ServiceProxyBase::onPropChangeUpdate(const CSMessagePtr& msg)
{
    std::vector<decltype(RegEntry::callback)> callbacks;

    {
        auto lk = _registerEntriesMap.a_lock();
        auto it = _registerEntriesMap->find(msg->operationID());
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

void ServiceProxyBase::onRequestResult(const CSMessagePtr& msg)
{
    decltype (RegEntry::callback) callback;
    {
        auto lk = _requestEntriesMap.a_lock();
        auto it = _requestEntriesMap->find(msg->operationID());
        if(it != _requestEntriesMap->end())
        {
            auto& regEntries = it->second;
            auto itRegEntry = regEntries.begin();
            for(; itRegEntry != regEntries.end(); ++itRegEntry)
            {
                if(itRegEntry->requestID == msg->requestID())
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

void ServiceProxyBase::onRequestSyncResult(const CSMessagePtr &msg)
{
    if(auto resultPromise = removeSyncRegEntry(RegID{ msg->requestID(), msg->operationID() }))
    {
        resultPromise->set_value(msg);
    }
    else
    {
        thafWarn("Could not find RegEntry for operationID [" << msg->operationID() << "] and requestID [" << msg->requestID() << "]");
    }
}

void ServiceProxyBase::abortAllSyncRequest()
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

bool ServiceProxyBase::sendMessageToServer(const CSMessagePtr &outgoingMsg)
{
    return (_client->sendMessageToServer(outgoingMsg) == DataTransmissionErrorCode::Success);
}

std::shared_ptr<std::future<CSMessagePtr > > ServiceProxyBase::storeSyncRegEntry(const CSMessagePtr &outgoingMsg, RegID &regID)
{
    regID.requestID = _idMgr.allocateNewID();
    regID.opID = outgoingMsg->operationID();

    std::shared_ptr<std::future<CSMessagePtr > > resultFuture;

    { // using {block} to unlock the mutex of _syncRequestEntriesMap
        auto lk = _syncRequestEntriesMap.a_lock();
        auto& listOfRequests = (*_syncRequestEntriesMap)[outgoingMsg->operationID()];
        std::promise<CSMessagePtr> prm;
        resultFuture = std::make_shared<std::future<CSMessagePtr >>(prm.get_future());
        SyncRegEntry regEntry = { regID.requestID, std::move(prm) };
        listOfRequests.push_back(std::move(regEntry));
    }
    outgoingMsg->setRequestID(regID.requestID);
    return resultFuture;
}

std::shared_ptr<std::promise<CSMessagePtr >> ServiceProxyBase::removeSyncRegEntry(const RegID& regID)
{
    std::shared_ptr<std::promise<CSMessagePtr >> resultPromise;
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
                    resultPromise = std::make_shared<std::promise<CSMessagePtr > >(std::move(itRequestEntry->_msgPromise));
                    listOfRequests.erase(itRequestEntry);
                    break;
                }
            }
        }
    }
    return resultPromise;
}

RegID ServiceProxyBase::storeAndSendRequestToServer
    (
        RegEntriesMap& regEntriesMap,
        const CSMessagePtr& outgoingMsg,
        CSMessageHandlerCallback callback
        )
{
    RegID regID;
    auto totalSameRequests = storeRegEntry(regEntriesMap, outgoingMsg->operationID(), callback, regID);
    // we have Proxy to reduce the number of requests to server, then in case of Register, if many classes call to register to same property, then only one will be sent
    bool shouldSendToServer = (outgoingMsg->operationCode() == OpCode::Register && totalSameRequests == 1 ) ||
                              (totalSameRequests >= 1 && outgoingMsg->operationCode() != OpCode::Register);

    if(shouldSendToServer)
    {
        outgoingMsg->setRequestID(regID.requestID);
        if(!sendMessageToServer(outgoingMsg))
        {
            removeRegEntry(regEntriesMap, regID);
        }
    }
    return regID;
}

size_t ServiceProxyBase::storeRegEntry
    (RegEntriesMap& regInfoEntries,
     OpID propertyID,
     CSMessageHandlerCallback callback,
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

size_t ServiceProxyBase::removeRegEntry(RegEntriesMap &regInfoEntries, RegID &regID)
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



} // messaging
} // thaf
