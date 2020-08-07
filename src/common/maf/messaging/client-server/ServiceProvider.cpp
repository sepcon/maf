#include "ServiceProvider.h"

#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/CSError.h>
#include <maf/messaging/client-server/ServerIF.h>

#include <cassert>

#include "Request.h"

namespace maf {
namespace messaging {

bool ServiceProvider::onIncomingMessage(const CSMessagePtr &msg) {
  MAF_LOGGER_INFO(
      "ServiceProvider - Received Incoming Message: ",
      "\n\tCode                   : ", msg->operationCode(),
      "\n\tID                     : ", msg->operationID(),
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
      onRequestAborted(msg);
      break;
    default:
      handled = false;
      MAF_LOGGER_WARN("Unhandled OpCode: ", msg->operationCode());
      break;
  }
  return handled;
}

ServiceProvider::ServiceProvider(ServiceID sid, std::weak_ptr<ServerIF> server)
    : sid_{std::move(sid)}, server_(std::move(server)) {
  assert(server_.lock() && "Server must not be null");
}

ServiceProvider::~ServiceProvider() {
  removeAllRegisterInfo();
  invalidateAndRemoveAllRequests();
}

const ServiceID &ServiceProvider::serviceID() const { return sid_; }

ActionCallStatus ServiceProvider::respondToRequest(const CSMessagePtr &csMsg) {
  if (auto request = pickOutRequestInfo(csMsg)) {
    request->invalidate();
    return sendBackMessageToClient(csMsg);
  } else {
    return ActionCallStatus::FailedUnknown;
  }
}

ActionCallStatus ServiceProvider::setStatus(const OpID &propertyID,
                                            const CSPayloadIFPtr &newProperty) {
  std::lock_guard lock(propertyMap_);

  auto &currentProperty = (*propertyMap_)[propertyID];
  if (!currentProperty || !currentProperty->equal(newProperty.get())) {
    currentProperty = newProperty;
    return broadcast(propertyID, OpCode::StatusRegister, newProperty);
  } else {
    MAF_LOGGER_INFO("Don't set status of property `", propertyID,
                    "` due to unchanged!");
    return ActionCallStatus::NoAction;
  }
}

ActionCallStatus ServiceProvider::broadcastSignal(
    const OpID &signalID, const CSPayloadIFPtr &signal) {
  return broadcast(signalID, OpCode::SignalRegister, signal);
}

ActionCallStatus ServiceProvider::broadcast(const OpID &propertyID,
                                            OpCode opCode,
                                            const CSPayloadIFPtr &content) {
  using AddressList = std::vector<Address>;
  bool success = false;
  AddressList addresses;

  {  // locking _regEntriesMap block
    std::lock_guard lock(regEntriesMap_);
    for (const auto &[clientAddress, registeredPropertyIDs] : *regEntriesMap_) {
      if (registeredPropertyIDs.find(propertyID) !=
          registeredPropertyIDs.end()) {
        addresses.push_back(clientAddress);
      }
    }
  }  // locking _regEntriesMap block

  if (addresses.empty()) {
    MAF_LOGGER_WARN("There's no register for property: ", propertyID);
  } else {
    auto csMsg = createCSMessage(serviceID(), propertyID, opCode,
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
          MAF_LOGGER_WARN(
              "Failed to send message "
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

CSPayloadIFPtr ServiceProvider::getStatus(const OpID &propertyID) {
  std::lock_guard lock(propertyMap_);
  if (auto itStatus = propertyMap_->find(propertyID);
      itStatus != propertyMap_->end()) {
    return itStatus->second;
  } else {
    return {};
  }
}

Availability ServiceProvider::availability() const { return availability_; }

void ServiceProvider::startServing() {
  availability_ = Availability::Available;
  if (auto server = server_.lock()) {
    server->onServiceStatusChanged(serviceID(), Availability::Unavailable,
                                   Availability::Available);
  }
}

void ServiceProvider::stopServing() {
  availability_ = Availability::Unavailable;
  if (auto server = server_.lock()) {
    server->onServiceStatusChanged(serviceID(), Availability::Available,
                                   Availability::Unavailable);
  }
}

ActionCallStatus ServiceProvider::sendMessage(const CSMessagePtr &csMsg,
                                              const Address &toAddr) {
  if (auto server = server_.lock()) {
    return server->sendMessageToClient(csMsg, toAddr);
  } else {
    return ActionCallStatus::ReceiverUnavailable;
  }
}

ActionCallStatus ServiceProvider::sendBackMessageToClient(
    const CSMessagePtr &csMsg) {
  return sendMessage(csMsg, csMsg->sourceAddress());
}

void ServiceProvider::onStatusChangeRegister(const CSMessagePtr &msg) {
  // Do this for notifying status when changed
  saveRegisterInfo(msg);
  // Do this for server to update latest status for new registered client
  updateLatestStatus(msg);
}

void ServiceProvider::onStatusChangeUnregister(const CSMessagePtr &msg) {
  removeRegisterInfo(msg);
}

ServiceProvider::RequestPtr ServiceProvider::saveRequestInfo(
    const CSMessagePtr &msg) {
  RequestPtr request{new Request(msg, weak_from_this())};
  (*requestsMap_.atomic())[msg->operationID()].push_back(request);
  return request;
}

ServiceProvider::RequestPtr ServiceProvider::pickOutRequestInfo(
    const CSMessagePtr &msg) {
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

void ServiceProvider::invalidateAndRemoveAllRequests() {
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

void ServiceProvider::saveRegisterInfo(const CSMessagePtr &msg) {
  (*regEntriesMap_.atomic())[msg->sourceAddress()].insert(msg->operationID());
}

void ServiceProvider::removeRegisterInfo(const CSMessagePtr &msg) {
  (*regEntriesMap_.atomic())[msg->sourceAddress()].erase(msg->operationID());
}

void ServiceProvider::removeAllRegisterInfo() {
  regEntriesMap_.atomic()->clear();
}

void ServiceProvider::removeRegistersOfAddress(const Address &addr) {
  regEntriesMap_.atomic()->erase(addr);
}

void ServiceProvider::onRequestAborted(const CSMessagePtr &msg) {
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

void ServiceProvider::onActionRequest(const CSMessagePtr &msg) {
  if (auto handlerCallback = getRequestHandlerCallback(msg->operationID())) {
    handlerCallback(saveRequestInfo(msg));
  } else {
    MAF_LOGGER_ERROR("Not found handler for ActionRequest with OpID[",
                     msg->operationID(), "]");
    msg->setPayload(std::make_shared<CSError>("Handler unavailable!",
                                              CSErrorCode::HandlerUnavailable));
    sendBackMessageToClient(msg);
  }
}

void ServiceProvider::onClientGoesOff(const CSMessagePtr &msg) {
  regEntriesMap_.atomic()->erase(msg->sourceAddress());
}

bool ServiceProvider::registerRequestHandler(
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

bool ServiceProvider::unregisterRequestHandler(const OpID &opID) {
  return requestHandlerMap_.atomic()->erase(opID) != 0;
}

void ServiceProvider::updateLatestStatus(const CSMessagePtr &registerMsg) {
  if (auto currentStatus = getStatus(registerMsg->operationID())) {
    registerMsg->setPayload(currentStatus);
    sendBackMessageToClient(registerMsg);
  }
}

void ServiceProvider::onStatusGetRequest(const CSMessagePtr &getMsg) {
  if (auto status = getStatus(getMsg->operationID())) {
    getMsg->setPayload(std::move(status));
  } else {
    getMsg->setPayload({});
    MAF_LOGGER_WARN("Client requests unavailable status `",
                    getMsg->operationID(), "`");
  }
  sendBackMessageToClient(getMsg);
}

RequestHandlerFunction ServiceProvider::getRequestHandlerCallback(
    const OpID &opID) {
  std::lock_guard lock(requestHandlerMap_);
  if (auto itHandler = requestHandlerMap_->find(opID);
      itHandler != requestHandlerMap_->end()) {
    return itHandler->second;
  }
  return {};
}

}  // namespace messaging
}  // namespace maf
