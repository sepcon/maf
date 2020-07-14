#include "ServiceRequester.h"

#include "ServiceRequesterImpl.h"

namespace maf {
namespace messaging {

ServiceRequester::ServiceRequester(const ServiceID &sid,
                                   std::weak_ptr<ClientIF> client)
    : pImpl_{std::make_unique<ServiceRequesterImpl>(sid, std::move(client))} {}

const ServiceID &ServiceRequester::serviceID() const {
  return pImpl_->serviceID();
}

ServiceRequester::~ServiceRequester() = default;

Availability ServiceRequester::serviceStatus() const {
  return pImpl_->serviceStatus();
}

RegID ServiceRequester::registerStatus(const OpID &propertyID,
                                       CSPayloadProcessCallback callback,
                                       ActionCallStatus *callStatus) {
  return pImpl_->registerStatus(propertyID, std::move(callback), callStatus);
}

RegID ServiceRequester::registerSignal(const OpID &propertyID,
                                       CSPayloadProcessCallback callback,
                                       ActionCallStatus *callStatus) {
  return pImpl_->registerSignal(propertyID, std::move(callback), callStatus);
}

ActionCallStatus ServiceRequester::unregister(const RegID &regID) {
  return pImpl_->unregister(regID);
}

ActionCallStatus ServiceRequester::unregisterAll(const OpID &propertyID) {
  return pImpl_->unregisterAll(propertyID);
}

RegID ServiceRequester::sendRequestAsync(const OpID &opID,
                                         const CSPayloadIFPtr &msgContent,
                                         CSPayloadProcessCallback callback,
                                         ActionCallStatus *callStatus) {
  return pImpl_->sendRequestAsync(opID, msgContent, std::move(callback),
                                  callStatus);
}

CSPayloadIFPtr ServiceRequester::getStatus(const OpID &propertyID,
                                           ActionCallStatus *callStatus,
                                           RequestTimeoutMs timeout) {
  return pImpl_->getStatus(propertyID, callStatus, timeout);
}

ActionCallStatus ServiceRequester::getStatus(
    const OpID &propertyID, CSPayloadProcessCallback callback) {
  return pImpl_->getStatus(propertyID, std::move(callback));
}

CSPayloadIFPtr ServiceRequester::sendRequest(const OpID &opID,
                                             const CSPayloadIFPtr &msgContent,
                                             ActionCallStatus *callStatus,
                                             RequestTimeoutMs timeout) {
  return pImpl_->sendRequest(opID, msgContent, callStatus, timeout);
}

void ServiceRequester::abortAction(const RegID &regID,
                                   ActionCallStatus *callStatus) {
  return pImpl_->abortAction(regID, callStatus);
}

void ServiceRequester::registerServiceStatusObserver(
    ServiceStatusObserverPtr serviceStatusObserver) {
  pImpl_->registerServiceStatusObserver(std::move(serviceStatusObserver));
}

void ServiceRequester::unregisterServiceStatusObserver(
    const ServiceStatusObserverPtr &serviceStatusObserver) {
  pImpl_->unregisterServiceStatusObserver(serviceStatusObserver);
}

bool ServiceRequester::onIncomingMessage(const CSMessagePtr &csMsg) {
  return pImpl_->onIncomingMessage(csMsg);
}

void ServiceRequester::onServiceStatusChanged(const ServiceID &sid,
                                              Availability oldStatus,
                                              Availability newStatus) noexcept {
  pImpl_->onServiceStatusChanged(sid, oldStatus, newStatus);
}

}  // namespace messaging
}  // namespace maf
