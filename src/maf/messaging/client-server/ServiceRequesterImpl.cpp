#include "ServiceRequesterImpl.h"

#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/CSError.h>
#include <maf/messaging/client-server/ClientIF.h>

#define SET_PTR_VALUE(pErrorStore, errorValue)                                 \
  if (pErrorStore) {                                                           \
    *pErrorStore = errorValue;                                                 \
  }                                                                            \
  static_cast<void *>(nullptr)

#define SET_ERROR_AND_RETURN_IF(condition, pErrorStore, errorValue,            \
                                returnedValue)                                 \
  if (condition) {                                                             \
    SET_PTR_VALUE(pErrorStore, errorValue);                                    \
    return returnedValue;                                                      \
  }                                                                            \
  static_cast<void *>(nullptr)

namespace maf {

namespace messaging {

bool ServiceRequesterImpl::onIncomingMessage(const CSMessagePtr &csMsg) {
  bool handled = true;
  MAF_LOGGER_INFO("New Incoming message from server:"
                  "\n\t\t sid     = ",
                  csMsg->serviceID(), "\n\t\t opID    = ", csMsg->operationID(),
                  "\n\t\t opCode  = ", csMsg->operationCode());

  if (csMsg && csMsg->serviceID() == sid_) {
    switch (csMsg->operationCode()) {
    case OpCode::SignalRegister:
      onPropChangeUpdate(csMsg);
      break;
    case OpCode::StatusRegister: {
      onPropChangeUpdate(csMsg);
      cachePropertyStatus(csMsg->operationID(), csMsg->content());
    } break;

    case OpCode::Request:
    case OpCode::StatusGet:
      onRequestResult(csMsg);
      break;
    default:
      handled = false;
      MAF_LOGGER_ERROR("Invalid RESPONSE operation code, "
                       "then cannot match to any INPUT code "
                       "[",
                       csMsg->operationCode(), "]");
      break;
    }
  }
  return handled;
}

ServiceRequesterImpl::ServiceRequesterImpl(const ServiceID &sid,
                                           std::weak_ptr<ClientIF> client)
    : client_(std::move(client)), sid_(sid) {}

ServiceRequesterImpl::~ServiceRequesterImpl() {
  MAF_LOGGER_INFO("Clean up service requester of service id: ", sid_, "...");
  abortAllSyncRequest();
}

RegID ServiceRequesterImpl::sendRequestAsync(
    const OpID &opID, const CSMsgContentBasePtr &msgContent,
    CSMessageContentHandlerCallback callback, ActionCallStatus *callStatus) {
  SET_ERROR_AND_RETURN_IF(serviceUnavailable(), callStatus,
                          ActionCallStatus::ServiceUnavailable, {});

  return sendMessageAsync(opID, OpCode::Request, msgContent,
                          std::move(callback), callStatus);
}

void ServiceRequesterImpl::abortAction(const RegID &regID,
                                       ActionCallStatus *callStatus) {
  SET_ERROR_AND_RETURN_IF(!regID.valid(), callStatus,
                          ActionCallStatus::InvalidParam,
                          /*void*/
  );

  bool found = false;

  { // create {block} for releasing lock on _requestEntriesMap
    std::lock_guard lock(requestEntriesMap_);
    auto it = requestEntriesMap_->find(regID.opID);
    if (it != requestEntriesMap_->end()) {
      auto &listOfRequests = it->second;

      for (auto requestEntryIt = listOfRequests.begin();
           requestEntryIt != listOfRequests.end(); ++requestEntryIt) {
        if (requestEntryIt->requestID == regID.requestID) {
          listOfRequests.erase(requestEntryIt);
          found = true;
          break;
        }
      }
    }
  }

  if (found) {
    auto msg = this->createCSMessage(regID.opID, OpCode::Abort);
    msg->setRequestID(regID.requestID);
    auto status = sendMessageToServer(msg);

    if (status == ActionCallStatus::Success) {
      RegID::reclaimID(regID, idMgr_);
    }

    SET_PTR_VALUE(callStatus, status);
  }
}

void ServiceRequesterImpl::registerServiceStatusObserver(
    ServiceStatusObserverIFPtr serviceStatusObserver) {
  if (auto observer = serviceStatusObserver.lock()) {
    Availability currentServiceStatus;
    {
      // both getting serviceStatus and pushing observer
      // to list must be in critical section to make sure
      // observer will never be missed the latest status event
      std::lock_guard lock(serviceStatusObservers_);
      currentServiceStatus = serviceStatus();
      serviceStatusObservers_->push_back(std::move(serviceStatusObserver));
    }

    if (currentServiceStatus == Availability::Available) {
      // dont execute notification in critical section,
      // that might block other thread to update service status
      // when status has changed
      observer->onServiceStatusChanged(sid_, Availability::Unknown,
                                       Availability::Available);
    }
  }
}

void ServiceRequesterImpl::unregisterServiceStatusObserver(
    const ServiceStatusObserverIFPtr &serviceStatusObserver) {
  std::lock_guard lock(serviceStatusObservers_);
  serviceStatusObservers_->remove_if(
      [&serviceStatusObserver](const auto &obsv) {
        // if the observer is already destroyed, then doesnt keep it as well
        return !obsv.lock() || (obsv.lock() == serviceStatusObserver.lock());
      });
}

CSMsgContentBasePtr ServiceRequesterImpl::sendRequest(
    const OpID &opID, const CSMsgContentBasePtr &msgContent,
    ActionCallStatus *callStatus, RequestTimeoutMs timeout) {

  SET_ERROR_AND_RETURN_IF(serviceUnavailable(), callStatus,
                          ActionCallStatus::ServiceUnavailable, {});

  return sendMessageSync(opID, OpCode::Request, msgContent, callStatus,
                         timeout);
}

Availability ServiceRequesterImpl::serviceStatus() const noexcept {
  return serviceStatus_;
}

bool ServiceRequesterImpl::serviceUnavailable() const noexcept {
  return serviceStatus_ != Availability::Available;
}

RegID ServiceRequesterImpl::sendMessageAsync(
    const OpID &operationID, OpCode operationCode,
    const CSMsgContentBasePtr &msgContent,
    CSMessageContentHandlerCallback callback, ActionCallStatus *callStatus) {
  auto csMsg = this->createCSMessage(operationID, operationCode, msgContent);
  return storeAndSendRequestToServer(requestEntriesMap_, csMsg,
                                     std::move(callback), callStatus);
}

CSMsgContentBasePtr
ServiceRequesterImpl::sendMessageSync(const OpID &operationID, OpCode opCode,
                                      const CSMsgContentBasePtr &msgContent,
                                      ActionCallStatus *callStatus,
                                      RequestTimeoutMs timeout) {
  auto promsise = std::make_shared<std::promise<CSMsgContentBasePtr>>();
  syncRequestPromises_.atomic()->push_back(promsise);
  auto resultFuture = promsise->get_future();
  auto onSyncMsgCallback = [&promsise, this](const CSMsgContentBasePtr &msg) {
    removeRequestPromies(promsise);
    promsise->set_value(msg);
  };

  auto regID = sendMessageAsync(operationID, opCode, msgContent,
                                std::move(onSyncMsgCallback), callStatus);

  if (regID.valid()) {
    try {
      if (timeout == RequestTimeoutMs::max()) {
        return resultFuture.get();
      } else {
        if (resultFuture.wait_for(timeout) == std::future_status::ready) {
          if (auto msg = resultFuture.get()) {
            return msg;
          }
        } else {
          MAF_LOGGER_WARN("Request id: ", regID.requestID,
                          " has expired!, then request server to abort action");

          abortAction(regID, nullptr);

          SET_PTR_VALUE(callStatus, ActionCallStatus::Timeout);
        }
      }
    } catch (const std::exception &e) {
      SET_PTR_VALUE(callStatus, ActionCallStatus::FailedUnknown);
      MAF_LOGGER_ERROR(
          "Error while waiting for result from server(Exception): ", e.what());
    } catch (...) {
      SET_PTR_VALUE(callStatus, ActionCallStatus::FailedUnknown);
      MAF_LOGGER_ERROR("Unknown exception when sending sync request to server");
    }
  } else // failed to send request to server
  {
    removeRequestPromies(promsise);
  }

  return {};
}

void ServiceRequesterImpl::onServiceStatusChanged(const ServiceID &sid,
                                                  Availability oldStatus,
                                                  Availability newStatus) {

  MAF_LOGGER_INFO("Server status change from ", oldStatus, " to ", newStatus);
  if ((sid == sid_) && (newStatus != serviceStatus_)) {
    serviceStatus_ = newStatus;
    if (newStatus == Availability::Unavailable) {
      serviceStatus_ = Availability::Unavailable;
      abortAllSyncRequest();
      clearAllAsyncRequests();
      clearAllRegisterEntries();
    }
    forwardServiceStatusToObservers(sid, oldStatus, newStatus);
  }
}

void ServiceRequesterImpl::forwardServiceStatusToObservers(
    const ServiceID &sid, Availability oldStatus, Availability newStatus) {
  std::lock_guard lock(serviceStatusObservers_);
  for (auto itObsv = std::begin(*serviceStatusObservers_);
       itObsv != std::end(*serviceStatusObservers_);) {
    if (auto observer = itObsv->lock()) {
      observer->onServiceStatusChanged(sid, oldStatus, newStatus);
      ++itObsv;
    } else {
      itObsv = serviceStatusObservers_->erase(itObsv);
    }
  }
}

RegID ServiceRequesterImpl::registerNotification(
    const OpID &opID, OpCode opCode, CSMessageContentHandlerCallback callback,
    ActionCallStatus *callStatus) {
  SET_ERROR_AND_RETURN_IF(!callback, callStatus, ActionCallStatus::InvalidParam,
                          {});

  RegID regID;
  auto sameRegisterCount =
      storeRegEntry(registerEntriesMap_, opID, callback, regID);
  if (sameRegisterCount == 1) {
    auto registerMessage = createCSMessage(opID, opCode);

    registerMessage->setRequestID(regID.requestID);

    auto status = sendMessageToServer(registerMessage);
    if (status != ActionCallStatus::Success) {
      removeRegEntry(registerEntriesMap_, regID);
      regID.clear();
    }

    SET_PTR_VALUE(callStatus, status);
  } else if (opCode == OpCode::StatusRegister) {
    if (auto cachedProperty = getCachedProperty(opID)) {
      callback(cachedProperty);
    }
    SET_PTR_VALUE(callStatus, ActionCallStatus::Success);
  }

  return regID;
}

CSMessagePtr
ServiceRequesterImpl::createCSMessage(const OpID &opID, OpCode opCode,
                                      const CSMsgContentBasePtr &msgContent) {
  return messaging::createCSMessage(sid_, std::move(opID), std::move(opCode),
                                    RequestIDInvalid, msgContent);
}

RegID ServiceRequesterImpl::registerStatus(
    const OpID &propertyID, CSMessageContentHandlerCallback callback,
    ActionCallStatus *callStatus) {

  SET_ERROR_AND_RETURN_IF(serviceUnavailable(), callStatus,
                          ActionCallStatus::ServiceUnavailable, {});

  return registerNotification(propertyID, OpCode::StatusRegister,
                              std::move(callback), callStatus);
}

RegID ServiceRequesterImpl::registerSignal(
    const OpID &eventID, CSMessageContentHandlerCallback callback,
    ActionCallStatus *callStatus) {
  SET_ERROR_AND_RETURN_IF(serviceUnavailable(), callStatus,
                          ActionCallStatus::ServiceUnavailable, {});

  return registerNotification(eventID, OpCode::SignalRegister,
                              std::move(callback), callStatus);
}

ActionCallStatus ServiceRequesterImpl::unregisterBroadcast(const RegID &regID) {
  auto callstatus = ActionCallStatus::Success;

  if (serviceUnavailable()) {
    callstatus = ActionCallStatus::ServiceUnavailable;
  } else if (regID.valid()) {
    auto propertyID = regID.opID;
    auto totalRemainer = removeRegEntry(registerEntriesMap_, regID);
    if (totalRemainer == 0) {
      // send unregister if no one from client side interested
      // in this propertyID anymore
      sendMessageToServer(createCSMessage(propertyID, OpCode::Unregister));
      removeCachedProperty(propertyID);
    }
  } else {
    callstatus = ActionCallStatus::InvalidParam;
    MAF_LOGGER_WARN("Try to Unregister invalid RegID");
  }

  return callstatus;
}

ActionCallStatus
ServiceRequesterImpl::unregisterBroadcastAll(const OpID &propertyID) {
  auto callstatus = ActionCallStatus::Success;
  if (serviceUnavailable()) {
    callstatus = ActionCallStatus::ServiceUnavailable;
  } else {
    registerEntriesMap_.atomic()->erase(propertyID);
    sendMessageToServer(createCSMessage(propertyID, OpCode::Unregister));
    removeCachedProperty(propertyID);
  }

  return callstatus;
}

CSMsgContentBasePtr
ServiceRequesterImpl::getStatus(const OpID &propertyID,
                                ActionCallStatus *callStatus,
                                RequestTimeoutMs timeout) {
  if (auto property = getCachedProperty(propertyID)) {
    return property;
  } else {
    SET_ERROR_AND_RETURN_IF(serviceUnavailable(), callStatus,
                            ActionCallStatus::ServiceUnavailable, {});

    return sendMessageSync(propertyID, OpCode::StatusGet, {}, callStatus,
                           timeout);
  }
}

ActionCallStatus
ServiceRequesterImpl::getStatus(const OpID &propertyID,
                                CSMessageContentHandlerCallback callback) {
  ActionCallStatus callstatus = ActionCallStatus::FailedUnknown;
  if (auto status = getCachedProperty(propertyID)) {
    callback(status);
    callstatus = ActionCallStatus::Success;
  } else {
    sendMessageAsync(propertyID, OpCode::StatusGet, {}, std::move(callback),
                     &callstatus);
  }
  return callstatus;
}

void ServiceRequesterImpl::onPropChangeUpdate(const CSMessagePtr &msg) {
  std::vector<decltype(RegEntry::callback)> callbacks;

  {
    std::lock_guard lock(registerEntriesMap_);
    auto it = registerEntriesMap_->find(msg->operationID());
    if (it != registerEntriesMap_->end()) {
      for (auto &regEntry : it->second) {
        callbacks.push_back(regEntry.callback);
      }
    }
  }

  auto content = msg->content();
  for (auto &callback : callbacks) {
    callback(content);
  }
}

void ServiceRequesterImpl::onRequestResult(const CSMessagePtr &msg) {
  decltype(RegEntry::callback) callback;
  {
    std::lock_guard lock(requestEntriesMap_);
    auto it = requestEntriesMap_->find(msg->operationID());
    bool found = false;
    if (it != requestEntriesMap_->end()) {
      auto &regEntries = it->second;
      for (auto itRegEntry = regEntries.begin(); itRegEntry != regEntries.end();
           ++itRegEntry) {
        if (itRegEntry->requestID == msg->requestID()) {
          callback = std::move(itRegEntry->callback);
          regEntries.erase(itRegEntry);
          found = true;
          break;
        }
      }
    }

    if (!found) {
      MAF_LOGGER_WARN("The request entry for request "
                      "OpID [",
                      msg->operationID(),
                      "] - "
                      "RequestiD[",
                      msg->requestID(),
                      "] "
                      "could not be found!");
    }
  }

  if (callback) {
    callback(msg->content());
  }
}

void ServiceRequesterImpl::abortAllSyncRequest() {
  int totalAborted = 0;
  std::lock_guard lock(syncRequestPromises_);
  for (auto &promise : *syncRequestPromises_) {
    promise->set_value({});
  }
  syncRequestPromises_->clear();

  if (totalAborted > 0) {
    MAF_LOGGER_INFO("Aborting ", totalAborted, " Sync requests!");
  }
}

void ServiceRequesterImpl::clearAllAsyncRequests() {
  requestEntriesMap_.atomic()->clear();
}

void ServiceRequesterImpl::clearAllRegisterEntries() {
  registerEntriesMap_.atomic()->clear();
}

ActionCallStatus
ServiceRequesterImpl::sendMessageToServer(const CSMessagePtr &outgoingMsg) {
  if (auto client = client_.lock()) {
    return client->sendMessageToServer(outgoingMsg);
  } else {
    return ActionCallStatus::ReceiverUnavailable;
  }
}

RegID ServiceRequesterImpl::storeAndSendRequestToServer(
    RegEntriesMap &regEntriesMap, const CSMessagePtr &outgoingMsg,
    CSMessageContentHandlerCallback callback, ActionCallStatus *callStatus) {
  RegID regID;

  storeRegEntry(regEntriesMap, outgoingMsg->operationID(), std::move(callback),
                regID);

  outgoingMsg->setRequestID(regID.requestID);
  auto status = sendMessageToServer(outgoingMsg);
  if (status != ActionCallStatus::Success) {
    removeRegEntry(regEntriesMap, regID);
    regID.clear();
  }

  SET_PTR_VALUE(callStatus, status);

  return regID;
}

size_t ServiceRequesterImpl::storeRegEntry(
    RegEntriesMap &regInfoEntries, const OpID &propertyID,
    CSMessageContentHandlerCallback callback, RegID &regID) {
  regID.requestID = idMgr_.allocateNewID();
  regID.opID = propertyID;

  std::lock_guard lock(regInfoEntries);
  auto &regEntries = (*regInfoEntries)[propertyID];
  regEntries.emplace_back(regID.requestID, std::move(callback));
  // means that already sent register for this propertyID to service
  return regEntries.size();
}

size_t ServiceRequesterImpl::removeRegEntry(RegEntriesMap &regInfoEntries,
                                            const RegID &regID) {
  std::lock_guard lock(regInfoEntries);
  auto it = regInfoEntries->find(regID.opID);
  if (it != regInfoEntries->end()) {
    it->second.erase(std::remove_if(it->second.begin(), it->second.end(),
                                    [&regID](const RegEntry &regEntry) {
                                      return regEntry.requestID ==
                                             regID.requestID;
                                    }),
                     it->second.end());
    RegID::reclaimID(regID, this->idMgr_);
    return it->second.size();
  } else {
    return 0;
  }
}

void ServiceRequesterImpl::removeRequestPromies(
    const std::shared_ptr<std::promise<CSMsgContentBasePtr>> &promise) {
  std::lock_guard lock(syncRequestPromises_);
  syncRequestPromises_->erase(std::find(syncRequestPromises_->begin(),
                                        syncRequestPromises_->end(), promise));
}

CSMsgContentBasePtr
ServiceRequesterImpl::getCachedProperty(const OpID &propertyID) const {
  std::lock_guard lock(propertiesCache_);
  if (auto itProp = propertiesCache_->find(propertyID);
      itProp != propertiesCache_->end()) {
    return itProp->second;
  }
  return {};
}

void ServiceRequesterImpl::cachePropertyStatus(const OpID &propertyID,
                                               CSMsgContentBasePtr property) {
  if (property) {
    propertiesCache_.atomic()->insert_or_assign(propertyID,
                                                std::move(property));
  }
}

void ServiceRequesterImpl::removeCachedProperty(const OpID &propertyID) {
  propertiesCache_.atomic()->erase(propertyID);
}

} // namespace messaging
} // namespace maf
