#include <maf/messaging/client-server/ClientInterface.h>
#include <maf/messaging/client-server/CSContentError.h>
#include <maf/logging/Logger.h>
#include "ServiceRequesterImpl.h"

#define mc_maf_throw_error(condition, explanation, errorCode)                  \
    if(condition)                                                              \
    {                                                                          \
        throw std::make_shared<CSContentError>(                                  \
            explanation,                                                       \
            errorCode                                                          \
        );                                                                     \
    }

#define mc_maf_set_ptr_value(pErrorStore, errorValue)                          \
if(pErrorStore)                                                                \
{                                                                              \
    *pErrorStore = errorValue;                                                 \
} static_cast<void*>(nullptr)

#define mc_maf_set_error_and_return(                                           \
                            condition,                                         \
                            pErrorStore,                                       \
                            errorValue,                                        \
                            returnedValue                                      \
                            )                                                  \
    if(condition)                                                              \
    {                                                                          \
        mc_maf_set_ptr_value(pErrorStore, errorValue);                         \
        return returnedValue;                                                  \
    } static_cast      <void*>(nullptr)                                        \

namespace maf { using logging::Logger;
namespace messaging {

bool ServiceRequesterImpl::onIncomingMessage(const CSMessagePtr &csMsg)
{
    bool handled = true;
    Logger::info(
        "New Incoming message from server:"
        "\n\t\t sid     = [", csMsg->serviceID(),       "]"
        "\n\t\t opID    = [", csMsg->operationID(),     "]"
        "\n\t\t opCode  = [", csMsg->operationCode(),   "]"
        );

    if(csMsg && csMsg->serviceID() == _sid)
    {
        switch (csMsg->operationCode()) {
        case OpCode::StatusRegister:
        case OpCode::SignalRegister:
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
                "then cannot match to any INPUT code "
                "[", csMsg->operationCode(), "]"
                );
            break;
        }
    }
    return handled;
}


ServiceRequesterImpl::ServiceRequesterImpl(
    const ServiceID& sid,
    std::weak_ptr<ClientInterface> client
    ) : _client(std::move(client)), _sid(sid)
{
}

ServiceRequesterImpl::~ServiceRequesterImpl()
{
    abortAllSyncRequest();
}

RegID ServiceRequesterImpl::sendRequestAsync(
    const OpID& opID,
    const CSMsgContentBasePtr &msgContent,
    CSMessageContentHandlerCallback callback,
    ActionCallStatus* callStatus
    )
{
    mc_maf_set_error_and_return(
        serviceUnavailable(),
        callStatus,
        ActionCallStatus::ServiceUnavailable,
        {} );

    return sendMessageAsync(
        opID,
        OpCode::Request,
        msgContent,
        std::move(callback),
        callStatus
        );
}

void ServiceRequesterImpl::abortAction(
    const RegID &regID,
    ActionCallStatus* callStatus
    )
{
    mc_maf_set_error_and_return(
        !regID.valid(),
        callStatus,
        ActionCallStatus::InvalidParam,
        /*void*/
        );

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
        auto status = sendMessageToServer(msg);

        if(status == ActionCallStatus::Success)
        {
            RegID::reclaimID(regID, _idMgr);
        }

        mc_maf_set_ptr_value(callStatus, status);
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

CSMsgContentBasePtr ServiceRequesterImpl::sendRequest(
    const OpID& opID,
    const CSMsgContentBasePtr &msgContent,
    unsigned long maxWaitTimeMs,
    ActionCallStatus* callStatus
    )
{
    mc_maf_set_error_and_return(
        serviceUnavailable(),
        callStatus,
        ActionCallStatus::ServiceUnavailable,
        {} );

    return sendMessageSync(
        opID,
        OpCode::Request,
        msgContent,
        maxWaitTimeMs,
        callStatus
        );
}

Availability ServiceRequesterImpl::serviceStatus() const
{
    return _serviceStatus;
}

bool ServiceRequesterImpl::serviceUnavailable() const
{
    return _serviceStatus != Availability::Available;
}

RegID ServiceRequesterImpl::sendMessageAsync(
    const OpID& operationID,
    OpCode operationCode,
    const CSMsgContentBasePtr &msgContent,
    CSMessageContentHandlerCallback callback,
    ActionCallStatus* callStatus
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
        std::move(callback),
        callStatus
        );
}

CSMsgContentBasePtr ServiceRequesterImpl::sendMessageSync(
    const OpID& operationID,
    OpCode opCode,
    const CSMsgContentBasePtr &msgContent,
    unsigned long maxWaitTimeMs,
    ActionCallStatus* callStatus
    )
{
    auto promsise = std::make_shared<std::promise<CSMsgContentBasePtr>>();
    _syncRequestPromises.atomic()->push_back(promsise);
    auto resultFuture = promsise->get_future();
    auto onSyncMsgCallback =
        [&promsise, this](const CSMsgContentBasePtr& msg ) {
        removeRequestPromies(promsise);
        promsise->set_value(msg);
    };

    auto regID = sendMessageAsync(
        operationID,
        opCode,
        msgContent,
        std::move(onSyncMsgCallback),
        callStatus
        );

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
            }
            else
            {
                Logger::warn(
                    "Request id: " ,
                    regID.requestID ,
                    " has expired!, then request server to abort action"
                    );

                abortAction(regID, nullptr);

                mc_maf_set_ptr_value(callStatus, ActionCallStatus::Timeout);
            }
        }
        catch(const std::exception& e)
        {
            mc_maf_set_ptr_value(callStatus, ActionCallStatus::FailedUnknown);
            Logger::error(
                    "Error while waiting for result from server(Exception): ",
                    e.what()
                );
        }
        catch(...)
        {
            mc_maf_set_ptr_value(callStatus, ActionCallStatus::FailedUnknown);
            Logger::error(
                "Unknown exception when sending sync request to server");
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
    const ServiceID& sid,
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
    const ServiceID& sid,
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

RegID ServiceRequesterImpl::registerNotification(
    const OpID& opID,
    OpCode opCode,
    CSMessageContentHandlerCallback callback,
    ActionCallStatus* callStatus
    )
{

    mc_maf_set_error_and_return(
        !callback,
        callStatus,
        ActionCallStatus::InvalidParam,
        {} );


    RegID regID;
    auto sameRegisterCount = storeRegEntry(
        _registerEntriesMap,
        opID,
        callback,
        regID
        );
    if(sameRegisterCount == 1)
    {
        auto registerMessage = createCSMessage(
            opID,
            opCode
            );

        registerMessage->setRequestID(regID.requestID);

        auto status = sendMessageToServer(registerMessage);
        if( status != ActionCallStatus::Success )
        {
            removeRegEntry(_registerEntriesMap, regID);
            regID.clear();
        }

        mc_maf_set_ptr_value(callStatus, status);
    }
    else if( opCode == OpCode::StatusRegister )
    {
        if( auto cachedProperty = getCachedProperty(opID) )
        {
            callback(cachedProperty);
        }
        mc_maf_set_ptr_value(callStatus, ActionCallStatus::Success);
    }

    return regID;
}

CSMessagePtr ServiceRequesterImpl::createCSMessage(
    const OpID& opID,
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
    const OpID& propertyID,
    CSMessageContentHandlerCallback callback,
    ActionCallStatus *callStatus
    )
{
    mc_maf_set_error_and_return(
        serviceUnavailable(),
        callStatus,
        ActionCallStatus::ServiceUnavailable,
        {} );

    return registerNotification(
        propertyID,
        OpCode::StatusRegister,
        std::move(callback),
        callStatus
        );
}

RegID ServiceRequesterImpl::registerSignal(
    const OpID& eventID,
    CSMessageContentHandlerCallback callback,
    ActionCallStatus* callStatus
    )
{
    mc_maf_set_error_and_return(
        serviceUnavailable(),
        callStatus,
        ActionCallStatus::ServiceUnavailable,
        {} );

    return registerNotification(
        eventID,
        OpCode::SignalRegister,
        std::move(callback),
        callStatus
        );
}

ActionCallStatus ServiceRequesterImpl::unregisterStatus(const RegID &regID)
{
    auto callstatus = ActionCallStatus::Success;

    if(serviceUnavailable())
    {
        callstatus = ActionCallStatus::ServiceUnavailable;
    }
    else if(regID.valid())
    {
        auto propertyID = regID.opID;
        auto totalRemainer = removeRegEntry(_registerEntriesMap, regID);
        if(totalRemainer == 0)
        {
            // send unregister if no one from client side interested
            // in this propertyID anymore
            sendMessageToServer(
                createCSMessage(propertyID, OpCode::Unregister)
                );
            removeCachedProperty(propertyID);
        }
    }
    else
    {
        callstatus = ActionCallStatus::InvalidParam;
        Logger::warn("Try to Unregister invalid RegID");
    }

    return callstatus;
}

ActionCallStatus ServiceRequesterImpl::unregisterStatusAll(const OpID& propertyID)
{
    auto callstatus = ActionCallStatus::Success;
    if(serviceUnavailable())
    {
        callstatus = ActionCallStatus::ServiceUnavailable;
    }
    else
    {
        _registerEntriesMap.atomic()->erase(propertyID);
        sendMessageToServer(createCSMessage(propertyID, OpCode::Unregister));
        removeCachedProperty(propertyID);
    }

    return callstatus;
}

CSMsgContentBasePtr ServiceRequesterImpl::getStatus(
    const OpID& propertyID,
    unsigned long maxWaitTimeMs,
    ActionCallStatus* callStatus
    )
{
    if(auto property = getCachedProperty(propertyID))
    {
        return property;
    }
    else
    {
        mc_maf_set_error_and_return(
            serviceUnavailable(),
            callStatus,
            ActionCallStatus::ServiceUnavailable,
            {} );

        return sendMessageSync(
            propertyID,
            OpCode::StatusGet,
            {},
            maxWaitTimeMs,
            callStatus
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
            for(
                auto itRegEntry = regEntries.begin();
                itRegEntry != regEntries.end();
                ++itRegEntry
                )
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
        Logger::warn(
            "The request entry for request "
            "OpID [",       msg->operationID(),     "] - "
            "RequestiD[" ,  msg->requestID(),       "] "
            "could not be found!"
            );
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

void ServiceRequesterImpl::clearAllAsyncRequests()
{
    _requestEntriesMap.atomic()->clear();
}

void ServiceRequesterImpl::clearAllRegisterEntries()
{
    _registerEntriesMap.atomic()->clear();
}

ActionCallStatus ServiceRequesterImpl::sendMessageToServer(
    const CSMessagePtr &outgoingMsg
    )
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
    CSMessageContentHandlerCallback callback,
    ActionCallStatus* callStatus
    )
{
    RegID regID;
    if(callback)
    {
        storeRegEntry(
            regEntriesMap,
            outgoingMsg->operationID(),
            std::move(callback),
            regID
            );
    }

    outgoingMsg->setRequestID(regID.requestID);

    auto status = sendMessageToServer(outgoingMsg);
    if( status != ActionCallStatus::Success )
    {
        removeRegEntry(regEntriesMap, regID);
        regID.clear();
    }

    mc_maf_set_ptr_value(callStatus, status);

    return regID;
}

size_t ServiceRequesterImpl::storeRegEntry(
    RegEntriesMap& regInfoEntries,
    const OpID& propertyID,
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

size_t ServiceRequesterImpl::removeRegEntry(
    RegEntriesMap &regInfoEntries,
    const RegID &regID
    )
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
                    [&regID](const RegEntry& regEntry) {
                        return regEntry.requestID == regID.requestID;
                    }
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

CSMsgContentBasePtr ServiceRequesterImpl::getCachedProperty(
    const OpID& propertyID
    ) const
{
    std::lock_guard lock(_propertiesCache);
    if(auto itProp = _propertiesCache->find(propertyID);
        itProp != _propertiesCache->end())
    {
        return itProp->second;
    }
    return {};
}

void ServiceRequesterImpl::cachePropertyStatus(
    const OpID& propertyID,
    CSMsgContentBasePtr property
    )
{
    if(property)
    {
        _propertiesCache.atomic()->insert_or_assign(
            propertyID,
            std::move(property)
            );
    }
}

void ServiceRequesterImpl::removeCachedProperty(const OpID& propertyID)
{
    _propertiesCache.atomic()->erase(propertyID);
}



} // messaging
} // maf
