#include "ClientBase.h"
#include <cassert>
#include <maf/logging/Logger.h>
#include "ServiceRequester.h"

namespace maf {

namespace messaging {

void ClientBase::onServerStatusChanged(Availability oldStatus,
                                       Availability newStatus) noexcept {
  if (newStatus != Availability::Available) {
    _serviceStatusMap.atomic()->clear();
    std::lock_guard lock(_requestersMap);
    for (auto &[sid, requester] : *_requestersMap) {
      requester->onServiceStatusChanged(sid, oldStatus, newStatus);
    }
  }
}

void ClientBase::onServiceStatusChanged(const ServiceID &sid,
                                        Availability oldStatus,
                                        Availability newStatus) noexcept {
  MAF_LOGGER_INFO("Client receives service status update from server: [", sid,
                  "]: ", oldStatus, "-->", newStatus);

  (*_serviceStatusMap.atomic())[sid] = newStatus;
  std::lock_guard lock(_requestersMap);
  auto itProxy = _requestersMap->find(sid);
  if (itProxy != _requestersMap->end()) {
    itProxy->second->onServiceStatusChanged(sid, oldStatus, newStatus);
  } else {
    MAF_LOGGER_WARN("There's no proxy for this service id: ", sid);
  }
}

bool ClientBase::hasServiceRequester(const ServiceID &sid) {
  return _requestersMap.atomic()->count(sid) != 0;
}

bool ClientBase::onIncomingMessage(const CSMessagePtr &msg) {
  if (msg->operationCode() == OpCode::ServiceStatusUpdate &&
      msg->serviceID() != ServiceIDInvalid) {
    MAF_LOGGER_INFO("Receive Service status update from server: sid[",
                    msg->serviceID(), "]-status[", msg->operationID(), "]");
    if (msg->operationID() == OpID_ServiceAvailable) {
      storeServiceStatus(msg->serviceID(), Availability::Available);
      onServiceStatusChanged(msg->serviceID(), Availability::Unavailable,
                             Availability::Available);
    } else if (msg->operationID() == OpID_ServiceUnavailable) {
      storeServiceStatus(msg->serviceID(), Availability::Unavailable);
      onServiceStatusChanged(msg->serviceID(), Availability::Available,
                             Availability::Unavailable);
    }
    return true;
  } else {
    auto requester = ServiceRequesterIFPtr{};
    {
      std::lock_guard lock(_requestersMap);
      if (auto itProxy = _requestersMap->find(msg->serviceID());
          itProxy != _requestersMap->end()) {
        requester = itProxy->second;
      }
    }

    if (requester) {
      return requester->onIncomingMessage(msg);
    }

    return false;
  }
}

void ClientBase::storeServiceStatus(const ServiceID &sid, Availability status) {
  (*_serviceStatusMap.atomic())[sid] = status;
}

ServiceRequesterIFPtr ClientBase::getServiceRequester(const ServiceID &sid) {
  std::lock_guard lock(_requestersMap);

  if (auto itRequester = _requestersMap->find(sid);
      itRequester != _requestersMap->end()) {
    return itRequester->second;
  } else {
    assert(shared_from_this());
    ServiceRequesterIFPtr requester =
        std::make_shared<ServiceRequester>(sid, weak_from_this());
    std::lock_guard lock(_serviceStatusMap);
    auto itServiceStatus = _serviceStatusMap->find(sid);
    if (itServiceStatus != _serviceStatusMap->end()) {
      requester->onServiceStatusChanged(itServiceStatus->first,
                                        Availability::Unknown,
                                        itServiceStatus->second);
    }
    _requestersMap->emplace(sid, requester);
    return requester;
  }
}

Availability ClientBase::getServiceStatus(const ServiceID &sid) {
  std::lock_guard lock(_serviceStatusMap);
  auto itStatus = _serviceStatusMap->find(sid);
  if (itStatus != _serviceStatusMap->end()) {
    return itStatus->second;
  } else {
    return Availability::Unavailable;
  }
}

bool ClientBase::init(const Address &, long long) { return true; }

bool ClientBase::deinit() {
  _requestersMap.atomic()->clear();
  std::lock_guard lock(_serviceStatusMap);
  for (auto &[serviceID, status] : *_serviceStatusMap) {
    sendMessageToServer(createCSMessage(serviceID, OpIDInvalid,
                                        OpCode::UnregisterServiceStatus));
  }
  _serviceStatusMap->clear();
  return true;
}

} // namespace messaging
} // namespace maf
