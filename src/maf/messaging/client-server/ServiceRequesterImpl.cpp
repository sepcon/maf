#include <maf/messaging/client-server/ClientInterface.h>
#include <maf/logging/Logger.h>
#include "ServiceRequesterImpl.h"


namespace maf { using logging::Logger;
namespace messaging {

bool ServiceRequesterImpl::onIncomingMessage(const CSMessagePtr &csMsg)
{
    bool handled = true;
    Logger::info("New Incoming message from server: sid[" ,
                 csMsg->serviceID() ,  "]-opID[" ,
                 csMsg->operationID() ,  "]-opCode[" ,
                 csMsg->operationCode() ,  "]"
                 );
    if(csMsg && csMsg->serviceID() == _sid)
    {
        switch (csMsg->operationCode()) {
        case OpCode::Register:
            onPropChangeUpdate(csMsg);
            break;
        case OpCode::Request:
        case OpCode::StatusGet:
            onRequestResult(csMsg);
            break;
        default:
            handled = false;
            Logger::error(
                "Invalid RESPONSE operation code, "
                "then cannot match to any REQUEST code[" ,
                csMsg->operationCode() ,
                "]"
                );
            break;
        }
    }
    return handled;
}


ServiceRequesterImpl::ServiceRequesterImpl(
    ServiceID sid,
    std::weak_ptr<ClientInterface> client
    ) : _client(std::move(client)), _sid(sid)
{
}

ServiceRequesterImpl::~ServiceRequesterImpl()
{
    abortAllSyncRequest();
}

RegID ServiceRequesterImpl::requestActionAsync(OpID opID,
                                                   const CSMsgContentBasePtr &msgContent,
                                                   CSMessageContentHandlerCallback callback
                                                   )
{
    return sendMessageAsync(
        opID,
        OpCode::Request,
        msgContent,
        std::move(callback)
        );
}

void ServiceRequesterImpl::abortAction(const RegID &regID)
{
    if(regID.opID != OpIDInvalid)
    {
        bool found = false;
        { // create {block} for releasing lock on _requestEntriesMap
            std::lock_guard lock(_requestEntriesMap);
            auto it = _requestEntriesMap->find(regID.opID);
            if(it != _requestEntriesMap->end())
            {
                auto& listOfRequests = it->second;

                for(
                    auto requestEntryIt = listOfRequests.begin();
                    requestEntryIt != listOfRequests.end();
                    ++requestEntryIt
                    )
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
            sendMessageToServer(msg);
            RegID::reclaimID(regID, _idMgr);
        }
    }
}

void ServiceRequesterImpl::addServiceStatusObserver(
    std::weak_ptr<ServiceStatusObserverInterface> serviceStatusObserver
    )
{
    _serviceStatusObservers.atomic()->push_back(
        std::move(serviceStatusObserver)
        );
}

void ServiceRequesterImpl::removeServiceStatusObserver(
    const std::weak_ptr<ServiceStatusObserverInterface>& serviceStatusObserver
    )
{
    std::lock_guard lock(_serviceStatusObservers);
    _serviceStatusObservers->remove_if(
        [&serviceStatusObserver](const auto& obsv) {
            return obsv.lock() == serviceStatusObserver.lock();
        }
        );
}

CSMsgContentBasePtr ServiceRequesterImpl::requestAction(
    OpID opID,
    const CSMsgContentBasePtr &msgContent,
    unsigned long maxWaitTimeMs
    )
{
    return sendMessageSync(
        opID,
        OpCode::Request,
        msgContent,
        maxWaitTimeMs
        );
}

Availability ServiceRequesterImpl::serviceStatus() const
{
    return _serviceStatus;
}

RegID ServiceRequesterImpl::sendMessageAsync(
    OpID operationID,
    OpCode operationCode,
    const CSMsgContentBasePtr &msgContent,
    CSMessageContentHandlerCallback callback
    )
{
    auto csMsg = this->createCSMessage(
        operationID,
        operationCode,
        msgContent
        );
    return storeAndSendRequestToServer(
        _requestEntriesMap,
        csMsg,
        std::move(callback)
        );
}

CSMsgContentBasePtr ServiceRequesterImpl::sendMessageSync(
    OpID operationID,
    OpCode opCode,
    const CSMsgContentBasePtr &msgContent,
    unsigned long maxWaitTimeMs
    )
{
    auto promsise = std::make_shared<std::promise<CSMsgContentBasePtr>>();
    _syncRequestPromises.atomic()->push_back(promsise);
    auto resultFuture = promsise->get_future();
    auto regID = sendMessageAsync(
        operationID,
        opCode,
        msgContent,
        [&promsise, this](const CSMsgContentBasePtr& msg ) {
            removeRequestPromies(promsise);
            promsise->set_value(msg);
        });

    if(regID.valid())
    {
        try
        {
            if(resultFuture.wait_for(std::chrono::milliseconds(maxWaitTimeMs))
                == std::future_status::ready )
            {
                if(auto msg = resultFuture.get())
                {
                    return msg;
                }
                else
                {
                    Logger::warn(
                        "Request id: " ,
                        regID.requestID ,
                        " has been cancelled"
                        );
                }
            }
            else
            {
                abortAction(regID);
            }
        }
        catch(const std::exception& e)
        {
            Logger::error(
                "Error while waiting for result from server(Exception): " ,
                e.what()
                );
        }
        catch(...)
        {
            Logger::error("Unknown exception when sending sync request to server");
        }
    }
    else // failed to send request to server
    {
        removeRequestPromies(promsise);
    }

    return {};
}

void ServiceRequesterImpl::onServerStatusChanged(
    Availability oldStatus,
    Availability newStatus)
{
    Logger::info("Server status changed from: ",
                 static_cast<int>(oldStatus),
                 " to ",
                 static_cast<int>(newStatus)
                 );
    if(newStatus == Availability::Unavailable)
    {
        _serviceStatus = Availability::Unavailable;
        abortAllSyncRequest();
        clearAllAsyncRequests();
        clearAllRegisterEntries();
    }

    forwardServerStatusToObservers(oldStatus, newStatus);
}

void ServiceRequesterImpl::onServiceStatusChanged(
    ServiceID sid,
    Availability oldStatus,
    Availability newStatus
    )
{
    if(sid == _sid && newStatus != _serviceStatus)
    {
        _serviceStatus = newStatus;
        forwardServiceStatusToObservers(sid, oldStatus, newStatus);
    }
}

void ServiceRequesterImpl::forwardServerStatusToObservers(
    Availability oldStatus,
    Availability newStatus
    )
{
    std::lock_guard lock(_serviceStatusObservers);
    for(auto itObsv = std::begin(*_serviceStatusObservers);
         itObsv != std::end(*_serviceStatusObservers);
         )
    {
        if(auto observer = itObsv->lock())
        {
            observer->onServerStatusChanged(oldStatus, newStatus);
            ++itObsv;
        }
        else
        {
            itObsv = _serviceStatusObservers->erase(itObsv);
        }
    }
}

void ServiceRequesterImpl::forwardServiceStatusToObservers(
    ServiceID sid,
    Availability oldStatus,
    Availability newStatus
    )
{
    std::lock_guard lock(_serviceStatusObservers);
    for(auto itObsv = std::begin(*_serviceStatusObservers);
         itObsv != std::end(*_serviceStatusObservers);
         )
    {
        if(auto observer = itObsv->lock())
        {
            observer->onServiceStatusChanged(sid, oldStatus, newStatus);
            ++itObsv;
        }
        else
        {
            itObsv = _serviceStatusObservers->erase(itObsv);
        }
    }
}

CSMessagePtr ServiceRequesterImpl::createCSMessage(
    OpID opID,
    OpCode opCode,
    const CSMsgContentBasePtr &msgContent
    )
{
    return messaging::createCSMessage(
        _sid,
        std::move(opID),
        std::move(opCode),
        RequestIDInvalid,
        msgContent
        );
}

RegID ServiceRequesterImpl::registerStatus(
    OpID propertyID,
    CSMessageContentHandlerCallback callback
    )
{
    RegID regID;
    if(callback && serviceStatus() == Availability::Available)
    {
        auto sameRegisterCount = storeRegEntry(
            _registerEntriesMap,
            propertyID,
            callback,
            regID
            );
        if(sameRegisterCount == 1)
        {
            auto registerMessage = createCSMessage(
                propertyID,
                OpCode::Register
                );

            registerMessage->setRequestID(regID.requestID);

            if(
                sendMessageToServer(registerMessage) != ActionCallStatus::Success
                )
            {
                removeRegEntry(_registerEntriesMap, regID);
                regID.clear();
            }
        }
        else if(auto cachedProperty = getCachedProperty(propertyID))
        {
            callback(cachedProperty);
        }
    }
    return regID;
}

void ServiceRequesterImpl::unregisterStatus(const RegID &regID)
{
    if(regID.valid())
    {
        auto propertyID = regID.opID;
        auto totalRemainer = removeRegEntry(_registerEntriesMap, regID);
        if(totalRemainer == 0)
        {
            // send unregister if no one from client side interested
            // in this propertyID anymore
            sendMessageToServer(createCSMessage(propertyID, OpCode::UnRegister));
            removeCachedProperty(propertyID);
        }
    }
    else
    {
        Logger::warn("Try to Unregister invalid RegID");
    }
}

void ServiceRequesterImpl::unregisterStatusAll(OpID propertyID)
{
    _registerEntriesMap.atomic()->erase(propertyID);
    sendMessageToServer(createCSMessage(propertyID, OpCode::UnRegister));
    removeCachedProperty(propertyID);
}

RegID ServiceRequesterImpl::getStatusAsync(
    OpID propertyID,
    CSMessageContentHandlerCallback callback
    )
{
    if(auto property = getCachedProperty(propertyID))
    {
        callback(property);
        return {};
    }
    else
    {
        return sendMessageAsync(
            propertyID,
            OpCode::StatusGet,
            CSMsgContentBasePtr{},
            std::move(callback)
            );
    }
}


CSMsgContentBasePtr ServiceRequesterImpl::getStatus(
    OpID propertyID,
    unsigned long maxWaitTimeMs
    )
{
    if(auto property = getCachedProperty(propertyID))
    {
        return property;
    }
    else
    {
        return sendMessageSync(
            propertyID,
            OpCode::StatusGet,
            {},
            maxWaitTimeMs
            );
    }
}

void ServiceRequesterImpl::onPropChangeUpdate(const CSMessagePtr& msg)
{
    std::vector<decltype(RegEntry::callback)> callbacks;

    {
        std::lock_guard lock(_registerEntriesMap);
        auto it = _registerEntriesMap->find(msg->operationID());
        if(it != _registerEntriesMap->end())
        {
            for(auto& regEntry : it->second)
            {
                callbacks.push_back(regEntry.callback);
            }
        }
    }

    auto content = msg->content();
    for(auto& callback : callbacks)
    {
        callback(content);
    }

    cachePropertyStatus(msg->operationID(), std::move(content));
}

void ServiceRequesterImpl::onRequestResult(const CSMessagePtr& msg)
{
    decltype (RegEntry::callback) callback;
    {
        std::lock_guard lock(_requestEntriesMap);
        auto it = _requestEntriesMap->find(msg->operationID());
        if(it != _requestEntriesMap->end())
        {
            auto& regEntries = it->second;
            for(auto itRegEntry = regEntries.begin(); itRegEntry != regEntries.end(); ++itRegEntry)
            {
                if(itRegEntry->requestID == msg->requestID())
                {
                    callback = std::move(itRegEntry->callback);
                    regEntries.erase(itRegEntry);
                    break;
                }
            }
        }
    }

    if(callback)
    {
        callback(msg->content());
    }
    else
    {
        Logger::warn("The request entry for requset OpID[" ,  msg->operationID() ,  "] - RequestiD[" ,  msg->requestID() , "] could not be found!");
    }
}

void ServiceRequesterImpl::abortAllSyncRequest()
{
    int totalAborted = 0;
    std::lock_guard lock(_syncRequestPromises);
    for(auto& promise : *_syncRequestPromises)
    {
        promise->set_value({});
    }
    _syncRequestPromises->clear();

    if(totalAborted > 0)
    {
        Logger::info("Aborting " ,  totalAborted ,  " Sync requests!");
    }
}

///
/// \brief ServiceRequesterImpl::clearAllRequests clears all requests, useful whenever server is unavalable
///
void ServiceRequesterImpl::clearAllAsyncRequests()
{
    _requestEntriesMap.atomic()->clear();
}

void ServiceRequesterImpl::clearAllRegisterEntries()
{
    _registerEntriesMap.atomic()->clear();
}

ActionCallStatus ServiceRequesterImpl::sendMessageToServer(const CSMessagePtr &outgoingMsg)
{
    if(auto client = _client.lock())
    {
        return client->sendMessageToServer(outgoingMsg);
    }
    else
    {
        return ActionCallStatus::ReceiverUnavailable;
    }
}

RegID ServiceRequesterImpl::storeAndSendRequestToServer(
    RegEntriesMap& regEntriesMap,
    const CSMessagePtr& outgoingMsg,
    CSMessageContentHandlerCallback callback
    )
{
    RegID regID;
    // dont need to check for callback empty here,
    // sometimes the one request doesnt need to care about return msg
    if(serviceStatus() == Availability::Available)
    {
        storeRegEntry(regEntriesMap, outgoingMsg->operationID(), std::move(callback), regID);
        outgoingMsg->setRequestID(regID.requestID);
        if(sendMessageToServer(outgoingMsg) != ActionCallStatus::Success)
        {
            removeRegEntry(regEntriesMap, regID);
            regID.clear();
        }
    }
    return regID;
}

size_t ServiceRequesterImpl::storeRegEntry(
    RegEntriesMap& regInfoEntries,
    OpID propertyID,
    CSMessageContentHandlerCallback callback,
    RegID &regID
    )
{
    regID.requestID = _idMgr.allocateNewID();
    regID.opID = propertyID;

    std::lock_guard lock(regInfoEntries);
    auto& regEntries = (*regInfoEntries)[propertyID];
    regEntries.emplace_back( regID.requestID, std::move(callback) );
    return regEntries.size(); //means that already sent register for this propertyID to service
}

size_t ServiceRequesterImpl::removeRegEntry(RegEntriesMap &regInfoEntries, const RegID &regID)
{
    std::lock_guard lock(regInfoEntries);
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

void ServiceRequesterImpl::removeRequestPromies(
    const std::shared_ptr<std::promise<CSMsgContentBasePtr> > &promise
    )
{
    std::lock_guard lock(_syncRequestPromises);
    _syncRequestPromises->erase(
        std::find(
            _syncRequestPromises->begin(),
            _syncRequestPromises->end(),
            promise
            )
        );
}

CSMsgContentBasePtr ServiceRequesterImpl::getCachedProperty(OpID propertyID) const
{
    std::lock_guard lock(_propertiesCache);
    if(auto itProp = _propertiesCache->find(propertyID);
        itProp != _propertiesCache->end())
    {
        return itProp->second;
    }
    return {};
}

void ServiceRequesterImpl::cachePropertyStatus(OpID propertyID, CSMsgContentBasePtr property)
{
    if(property)
    {
        _propertiesCache.atomic()->insert_or_assign(
            propertyID,
            std::move(property)
            );
    }
}

void ServiceRequesterImpl::removeCachedProperty(OpID propertyID)
{
    _propertiesCache.atomic()->erase(propertyID);
}



} // messaging
} // maf
