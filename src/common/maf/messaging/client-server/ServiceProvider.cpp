#include "ServiceProvider.h"
#include "ServiceProviderImpl.h"

namespace maf {
namespace messaging {

ServiceProvider::ServiceProvider(ServiceID sid,
                                 std::weak_ptr<ServerIF> server) {
  pImpl_ = std::make_unique<ServiceProviderImpl>(std::move(sid), this,
                                                 std::move(server));
}

ServiceProvider::~ServiceProvider() { pImpl_->stopServing(); }

const ServiceID &ServiceProvider::serviceID() const {
  return pImpl_->serviceID();
}

Availability ServiceProvider::availability() const {
  return pImpl_->availability();
}

bool ServiceProvider::registerRequestHandler(
    const OpID &opID, RequestHandlerFunction handlerFunction) {
  return pImpl_->registerRequestHandler(opID, std::move(handlerFunction));
}

bool ServiceProvider::unregisterRequestHandler(const OpID &opID) {
  return pImpl_->unregisterRequestHandler(opID);
}

ActionCallStatus ServiceProvider::respondToRequest(const CSMessagePtr &csMsg) {
  return pImpl_->respondToRequest(csMsg);
}

ActionCallStatus ServiceProvider::setStatus(const OpID &propertyID,
                                            const CSPayloadIFPtr &property) {
  return pImpl_->setStatus(propertyID, property);
}

ActionCallStatus
ServiceProvider::broadcastSignal(const OpID &signalID,
                                 const CSPayloadIFPtr &signal) {
  return pImpl_->broadcastSignal(signalID, signal);
}

CSPayloadIFPtr ServiceProvider::getStatus(const OpID &propertyID) {
  return pImpl_->getStatus(propertyID);
}

void ServiceProvider::startServing() { pImpl_->startServing(); }

void ServiceProvider::stopServing() { pImpl_->stopServing(); }

bool ServiceProvider::onIncomingMessage(const CSMessagePtr &csMsg) {
  return pImpl_->onIncomingMessage(csMsg);
}

} // namespace messaging
} // namespace maf
