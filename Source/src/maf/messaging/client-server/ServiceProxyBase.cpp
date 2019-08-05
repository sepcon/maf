#include "maf/messaging/client-server/ServiceProxyBase.h"
#include "maf/messaging/client-server/ClientInterface.h"
#include "maf/utils/debugging/Debug.h"


namespace maf {
namespace messaging {

bool ServiceProxyBase::onIncomingMessage(const CSMessagePtr &csMsg)
{
    bool handled = true;
    mafInfo("New Incoming message from server: sid[" << csMsg->serviceID() << "]-opID[" << csMsg->operationID() << "]-opCode[" << csMsg->operationCode() << "]");
    if(csMsg && csMsg->serviceID() == serviceID())
    {
        switch (csMsg->operationCode()) {
        case OpCode::StatusUpdate:
            onPropChangeUpdate(csMsg);
            break;
        case OpCode::RequestResultUpdate:
            onRequestResult(csMsg, false);
            break;
        case OpCode::RequestSyncResultUpdate:
        case OpCode::RequestSyncResultDone:
            onRequestSyncResult(csMsg);
            break;
        case OpCode::RequestResultDone:
            onRequestResult(csMsg, true);
            break;
        default:
            handled = false;
            mafErr("Invalid RESPONSE operation code, then cannot match to any REQUEST code[" << csMsg->operationCode() << "]");
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
    assert(msgContent && "Message content must not be null when passing to this function");
    return sendRequest(msgContent->operationID(), msgContent, callback);
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
            auto msg = this->createCSMessage(regID.opID, OpCode::Abort);
            msg->setRequestID(regID.requestID);
            _client->sendMessageToServer(msg);
            RegID::reclaimID(regID, _idMgr);
        }
    }
}

void ServiceProxyBase::sendAbortSyncRequest(const RegID &regID)
{
    pickOutSyncRegEntry(regID);
    _client->sendMessageToServer(this->createCSMessage(regID.opID, OpCode::Abort));
}

bool ServiceProxyBase::sendRequestSync(const CSMsgContentPtr &msgContent, CSMessageHandlerCallback callback, unsigned long maxWaitTimeMs)
{
    bool success = false;
    auto responseMsg = sendRequestSync(msgContent, maxWaitTimeMs);
    try
    {
       if(callback) callback(responseMsg);
        success = true;
    }
    catch (const std::exception& e)
    {
        mafErr("Exception when executing callback: " << e.what());
    }
    return success;
}

CSMessagePtr ServiceProxyBase::sendRequestSync(const CSMsgContentPtr &msgContent, unsigned long maxWaitTimeMs)
{
    assert(msgContent && "msgContent is not allowed to be null here");
    return sendRequestSync(msgContent->operationID(), msgContent, maxWaitTimeMs);
}

RegID ServiceProxyBase::sendRequest(OpID operationID, const CSMsgContentPtr &msgContent, CSMessageHandlerCallback callback)
{
    auto csMsg = this->createCSMessage(operationID, OpCode::Request, msgContent);
    return storeAndSendRequestToServer(_requestEntriesMap, csMsg, callback);
}

CSMessagePtr ServiceProxyBase::sendRequestSync(OpID operationID, const CSMsgContentPtr &msgContent, unsigned long maxWaitTimeMs)
{
    RegID regID;
    auto csMsg = this->createCSMessage(operationID, OpCode::RequestSync, msgContent);
    auto resultFuture = storeSyncRegEntry(csMsg, regID);
    if(resultFuture && sendMessageToServer(csMsg))
    {
        try
        {
            if(resultFuture->wait_for(std::chrono::milliseconds(maxWaitTimeMs)) == std::future_status::ready)
            {
                if(auto msg = resultFuture->get())
                {
                    return msg;
                }
                else
                {
                    mafWarn("Request id: " << regID.requestID << " has been cancelled");
                }
            }
            else
            {
                sendAbortSyncRequest(regID);
            }
        }
        catch(const std::exception& e)
        {
            mafErr("Error while waiting for result from server(Exception): " << e.what());
        }
        catch(...)
        {
            mafErr("Unknown exception when sending sync request to server");
        }
    }
    return {};
}

void ServiceProxyBase::onServerStatusChanged(Availability oldStatus, Availability newStatus)
{
    mafInfo("Server Status changed from: " << static_cast<int>(oldStatus) << " to " << static_cast<int>(newStatus));
    if(newStatus == Availability::Unavailable)
    {
        abortAllSyncRequest();
        clearAllAsyncRequests();
        clearAllRegisterEntries();
    }
}

void ServiceProxyBase::onServiceStatusChanged(ServiceID sid, Availability oldStatus, Availability newStatus)
{
    mafInfo("ServiceID [" << sid << "] has changed status from: " << static_cast<int>(oldStatus) << " to " << static_cast<int>(newStatus));
}

CSMessagePtr ServiceProxyBase::createCSMessage(OpID opID, OpCode opCode, const CSMsgContentPtr &msgContent)
{
    return messaging::createCSMessage(serviceID(), std::move(opID), std::move(opCode), RequestIDInvalid, msgContent);
}

RegID ServiceProxyBase::sendStatusChangeRegister(OpID propertyID, CSMessageHandlerCallback callback)
{
    constexpr bool forceSend = false;
    return storeAndSendRequestToServer
            (
                _registerEntriesMap,
                createCSMessage(propertyID, OpCode::Register),
                callback,
                forceSend
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
        mafWarn("Try to Unregister invalid RegID");
    }
}

void ServiceProxyBase::sendStatusChangeUnregisterAll(OpID propertyID)
{
    auto lock(_registerEntriesMap.pa_lock());
    _registerEntriesMap->erase(propertyID);
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

void ServiceProxyBase::onRequestResult(const CSMessagePtr& msg, bool done)
{
    bool foundRequest = false;
    decltype (RegEntry::callback) callback;
    {
        auto lk = _requestEntriesMap.a_lock();
        auto it = _requestEntriesMap->find(msg->operationID());
        if(it != _requestEntriesMap->end())
        {
            auto& regEntries = it->second;
            for(auto itRegEntry = regEntries.begin(); itRegEntry != regEntries.end(); ++itRegEntry)
            {
                if(itRegEntry->requestID == msg->requestID())
                {
					if (done)
					{
						callback = std::move(itRegEntry->callback);
						regEntries.erase(itRegEntry);
					}
					else
					{
						callback = itRegEntry->callback;
					}
                    foundRequest = true;
                    break;
                }
            }
        }
    }

    if(foundRequest)
    {
        if(callback)
        {
            callback(msg);
        }
    }
	else
	{
		mafWarn("The request entry for requset OpID[" << msg->operationID() << "] - RequestiD[" << msg->requestID() <<"] could not be found!");
	}
}

void ServiceProxyBase::onRequestSyncResult(const CSMessagePtr &msg)
{
    if(auto resultPromise = pickOutSyncRegEntry(RegID{ msg->requestID(), msg->operationID() }))
    {
        resultPromise->set_value(msg);
    }
    else
    {
        mafWarn("Could not find RegEntry for operationID [" << msg->operationID() << "] and requestID [" << msg->requestID() << "]");
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
        mafWarn("Aborting " << totalAborted << " Sync requests!");
    }
    _syncRequestEntriesMap->clear();
}

///
/// \brief ServiceProxyBase::clearAllRequests clears all requests, useful whenever server is unavalable
///
void ServiceProxyBase::clearAllAsyncRequests()
{
    auto lock(_requestEntriesMap.pa_lock());
    _requestEntriesMap->clear();
}

void ServiceProxyBase::clearAllRegisterEntries()
{
    auto lock(_registerEntriesMap.pa_lock());
    _registerEntriesMap->clear();
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

std::shared_ptr<std::promise<CSMessagePtr >> ServiceProxyBase::pickOutSyncRegEntry(const RegID& regID)
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
    (RegEntriesMap& regEntriesMap,
        const CSMessagePtr& outgoingMsg,
        CSMessageHandlerCallback callback
        , bool forceSend)
{
    RegID regID;
    auto totalSameRequests = storeRegEntry(regEntriesMap, outgoingMsg->operationID(), callback, regID);
    // we have Proxy to reduce the number of requests to server, then in case of Register, if many classes call to register to same property, then only one will be sent
    bool shouldSendToServer = forceSend || totalSameRequests == 1;

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
} // maf
