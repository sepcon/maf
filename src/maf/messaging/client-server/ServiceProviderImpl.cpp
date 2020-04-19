#include "ServiceProviderImpl.h"
#include "Request.h"
#include "ServiceProvider.h"
#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/CSError.h>
#include <maf/messaging/client-server/ServerIF.h>
#include <cassert>

namespace maf {
namespace messaging {

bool ServiceProviderImpl::onIncomingMessage(const CSMessagePtr &msg) {
  MAF_LOGGER_INFO(
      "Received Incoming Message  : ", "\n\tCode                   : ",
      msg->operationCode(), "\n\tID                     : ", msg->operationID(),
      "\n\tSenderAddress          : ", msg->sourceAddress().dump(2));

  bool handled = true;
  switch (msg->operationCode()) {
  case OpCode::StatusRegister:
    onStatusChangeRegister(msg);
    break;
  case OpCode::SignalRegister:
    saveRegisterInfo(msg);
    break;
  case OpCode::Unregister:
    onStatusChangeUnregister(msg);
    break;
  case OpCode::UnregisterServiceStatus:
    onClientGoesOff(msg);
    break;
  case OpCode::Request:
    onActionRequest(msg);
    break;
  case OpCode::StatusGet:
    onStatusGetRequest(msg);
    break;
  case OpCode::Abort:
    onAbortActionRequest(msg);
    break;
  default:
    handled = false;
    MAF_LOGGER_WARN("Unhandled OpCode: ", msg->operationCode());
    break;
  }
  return handled;
}

ServiceProviderImpl::ServiceProviderImpl(ServiceProvider *holder,
                                         std::weak_ptr<ServerIF> server)
    : server_(std::move(server)), delegator_(std::move(holder)) {
  assert(server_.lock() && "Server must not be null");
}

ServiceProviderImpl::~ServiceProviderImpl() {
  removeAllRegisterInfo();
  invalidateAndRemoveAllRequests();
}

ActionCallStatus
ServiceProviderImpl::respondToRequest(const CSMessagePtr &csMsg) {
  if (auto request = pickOutRequestInfo(csMsg)) {
    request->invalidate();
    return sendBackMessageToClient(csMsg);
  } else {
    return ActionCallStatus::FailedUnknown;
  }
}

ActionCallStatus
ServiceProviderImpl::setStatus(const OpID &propertyID,
                               const CSMsgContentBasePtr &property) {
  propertyMap_.atomic()->insert_or_assign(propertyID, property);

  return broadcast(propertyID, OpCode::StatusRegister, property);
}

ActionCallStatus
ServiceProviderImpl::broadcastSignal(const OpID &signalID,
                                     const CSMsgContentBasePtr &signal) {
  return broadcast(signalID, OpCode::SignalRegister, signal);
}

ActionCallStatus
ServiceProviderImpl::broadcast(const OpID &propertyID, OpCode opCode,
                               const CSMsgContentBasePtr &content) {

  using AddressList = std::vector<Address>;
  bool success = false;
  AddressList addresses;

  { // locking _regEntriesMap block
    std::lock_guard lock(regEntriesMap_);
    for (const auto &[clientAddress, registeredPropertyIDs] : *regEntriesMap_) {
      if (registeredPropertyIDs.find(propertyID) !=
          registeredPropertyIDs.end()) {
        addresses.push_back(clientAddress);
      }
    }
  } // locking _regEntriesMap block

  if (addresses.empty()) {
    MAF_LOGGER_WARN("There's no register for property: ", propertyID);
  } else {
    auto csMsg = createCSMessage(delegator_->serviceID(), propertyID, opCode,
                                 RequestIDInvalid, content);

    auto trySendToDestinations =
        [this, &csMsg](AddressList &addresses) -> AddressList {
      AddressList busyReceivers;
      for (const auto &addr : addresses) {
        auto errCode = sendMessage(csMsg, addr);
        if (errCode == ActionCallStatus::Success) {
          MAF_LOGGER_INFO("Sent message id: ", csMsg->operationID(),
                          " from server side!");
        } else if (errCode == ActionCallStatus::ReceiverBusy) {
          busyReceivers.emplace_back(std::move(addr));
        } else {
          this->removeRegistersOfAddress(addr);
          MAF_LOGGER_WARN("Failed to send message "
                          "[",
                          csMsg->operationID(),
                          "] "
                          "to client ",
                          addr.dump());
        }
      }
      return busyReceivers;
    };

    auto busyReceivers = trySendToDestinations(addresses);
    if (!busyReceivers.empty()) {
      // If someones are busy, try with them once
      MAF_LOGGER_WARN("Trying to send message to busy addresses once again!");
      busyReceivers = trySendToDestinations(busyReceivers);
    }

    // success when succeeded to send msg to at least one receiver
    success = (busyReceivers.size() != addresses.size());
  }
  return success ? ActionCallStatus::ReceiverUnavailable
                 : ActionCallStatus::Success;
}

CSMsgContentBasePtr ServiceProviderImpl::getStatus(const OpID &propertyID) {
  std::lock_guard lock(propertyMap_);
  if (auto itStatus = propertyMap_->find(propertyID);
      itStatus != propertyMap_->end()) {
    return itStatus->second;
  } else {
    return {};
  }
}

Availability ServiceProviderImpl::availability() const { return availability_; }

void ServiceProviderImpl::startServing() {
  availability_ = Availability::Available;
  if (auto server = server_.lock()) {
    server->registerServiceProvider(delegator_->shared_from_this());
  }
}

void ServiceProviderImpl::stopServing() {
  availability_ = Availability::Unavailable;
  if (auto server = server_.lock()) {
    server->unregisterServiceProvider(delegator_->serviceID());
  }
}

ActionCallStatus ServiceProviderImpl::sendMessage(const CSMessagePtr &csMsg,
                                                  const Address &toAddr) {
  if (auto server = server_.lock()) {
    return server->sendMessageToClient(csMsg, toAddr);
  } else {
    return ActionCallStatus::ReceiverUnavailable;
  }
}

ActionCallStatus
ServiceProviderImpl::sendBackMessageToClient(const CSMessagePtr &csMsg) {
  return sendMessage(csMsg, csMsg->sourceAddress());
}

void ServiceProviderImpl::onStatusChangeRegister(const CSMessagePtr &msg) {
  // Do this for notifying status when changed
  saveRegisterInfo(msg);
  // Do this for server to update latest status for new registered client
  updateLatestStatus(msg);
}

void ServiceProviderImpl::onStatusChangeUnregister(const CSMessagePtr &msg) {
  removeRegisterInfo(msg);
}

ServiceProviderImpl::RequestPtr
ServiceProviderImpl::saveRequestInfo(const CSMessagePtr &msg) {
  RequestPtr request{new Request(msg, delegator_->weak_from_this())};
  (*requestsMap_.atomic())[msg->operationID()].push_back(request);
  return request;
}

ServiceProviderImpl::RequestPtr
ServiceProviderImpl::pickOutRequestInfo(const CSMessagePtr &msg) {
  RequestPtr request;
  std::lock_guard lock(requestsMap_);
  auto itRequestList = requestsMap_->find(msg->operationID());
  if (itRequestList != requestsMap_->end()) {
    auto &requestList = itRequestList->second;
    for (auto itRequest = requestList.begin(); itRequest != requestList.end();
         ++itRequest) {
      RequestPtr &requestTmp = *itRequest;
      if (requestTmp->getRequestID() == msg->requestID()) {
        request = std::move(requestTmp);
        requestList.erase(itRequest);
        break;
      }
    }
  }
  return request;
}

void ServiceProviderImpl::invalidateAndRemoveAllRequests() {
  std::lock_guard lock(requestsMap_);
  for (auto &[opID, requests] : *requestsMap_) {
    for (auto &request : requests) {
      request->invalidate();
      if (auto abortCallback = request->getAbortCallback()) {
        abortCallback();
      }
    }
  }
  regEntriesMap_->clear();
}

void ServiceProviderImpl::saveRegisterInfo(const CSMessagePtr &msg) {
  (*regEntriesMap_.atomic())[msg->sourceAddress()].insert(msg->operationID());
}

void ServiceProviderImpl::removeRegisterInfo(const CSMessagePtr &msg) {
  (*regEntriesMap_.atomic())[msg->sourceAddress()].erase(msg->operationID());
}

void ServiceProviderImpl::removeAllRegisterInfo() {
  regEntriesMap_.atomic()->clear();
}

void ServiceProviderImpl::removeRegistersOfAddress(const Address &addr) {
  regEntriesMap_.atomic()->erase(addr);
}

void ServiceProviderImpl::onAbortActionRequest(const CSMessagePtr &msg) {
  if (auto request = pickOutRequestInfo(msg)) {
    // Invalidate request then later respond from request itself
    // will not cause any race condition.
    // Must be considered carefully for bug fixing later
    request->invalidate();
    if (auto abortCallback = request->getAbortCallback()) {
      abortCallback();
    }
  }
}

void ServiceProviderImpl::onActionRequest(const CSMessagePtr &msg) {
  if (auto handlerCallback = getRequestHandlerCallback(msg->operationID())) {
    handlerCallback(saveRequestInfo(msg));
  } else {
    MAF_LOGGER_ERROR("Not found handler for ActionRequest with OpID[",
                     msg->operationID(), "]");
    msg->setContent(std::make_shared<CSError>("Handler unavailable!",
                                              CSErrorCode::HandlerUnavailable));
    sendBackMessageToClient(msg);
  }
}

void ServiceProviderImpl::onClientGoesOff(const CSMessagePtr &msg) {
  regEntriesMap_.atomic()->erase(msg->sourceAddress());
}

bool ServiceProviderImpl::registerRequestHandler(
    const OpID &opID, RequestHandlerFunction handlerFunction) {
  if (handlerFunction) {
    auto [itInsertedPos, success] = requestHandlerMap_.atomic()->try_emplace(
        opID, std::move(handlerFunction));
    return success;
  } else {
    MAF_LOGGER_ERROR("Trying to set empty function as handler for OpID [", opID,
                     "]");
  }
  return false;
}

bool ServiceProviderImpl::unregisterRequestHandler(const OpID &opID) {
  return requestHandlerMap_.atomic()->erase(opID) != 0;
}

void ServiceProviderImpl::updateLatestStatus(const CSMessagePtr &registerMsg) {
  if (auto currentStatus = getStatus(registerMsg->operationID())) {
    registerMsg->setContent(currentStatus);
    sendBackMessageToClient(registerMsg);
  }
}

void ServiceProviderImpl::onStatusGetRequest(const CSMessagePtr &getMsg) {
  if (auto status = getStatus(getMsg->operationID())) {
    getMsg->setContent(std::move(status));
  } else {
    getMsg->setContent({});
    MAF_LOGGER_WARN("Client requests unavailable status `",
                    getMsg->operationID(), "`");
  }
  sendBackMessageToClient(getMsg);
}

RequestHandlerFunction
ServiceProviderImpl::getRequestHandlerCallback(const OpID &opID) {

  std::lock_guard lock(requestHandlerMap_);
  if (auto itHandler = requestHandlerMap_->find(opID);
      itHandler != requestHandlerMap_->end()) {
    return itHandler->second;
  }
  return {};
}

} // namespace messaging
} // namespace maf
