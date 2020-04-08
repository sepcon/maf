#ifndef MAF_MESSAGING_CLIENT_SERVER_STUB_IMPL_H
#define MAF_MESSAGING_CLIENT_SERVER_STUB_IMPL_H

#ifndef MAF_MESSAGING_CLIENT_SERVER_STUB_H
#include <maf/messaging/client-server/Stub.h>
#endif

#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/CSManager.h>

namespace maf {
namespace messaging {

using namespace paco;

template <class MTrait>
Stub<MTrait>::Stub(ProviderPtr provider, ExecutorPtr executor)
    : provider_(std::move(provider)), executor_{std::move(executor)} {}

template <class MTrait>
typename Stub<MTrait>::StubPtr
Stub<MTrait>::createStub(const ConnectionType &contype, const Address &addr,
                         const ServiceID &sid, ExecutorPtr executor) {
  if (auto provider =
          CSManager::instance().getServiceProvider(contype, addr, sid)) {
    return std::shared_ptr<Stub>{
        new Stub(std::move(provider), std::move(executor))};
  }

  return {};
}

template <class MTrait> const ServiceID &Stub<MTrait>::serviceID() const {
  return provider_->serviceID();
}

template <class MTrait>
template <class Status, AllowOnlyStatusT<MTrait, Status>>
ActionCallStatus
Stub<MTrait>::setStatus(const std::shared_ptr<Status> &status) {
  if (status) {
    auto propertyID = MTrait::template getOperationID<Status>();

    auto oldStatus = provider_->getStatus(propertyID);
    auto newStatus = MTrait::template encode(status);

    if (!newStatus->equal(oldStatus.get())) {
      return provider_->setStatus(propertyID, newStatus);
    } else {
      MAF_LOGGER_WARN(
          "Trying to set new status thats not different with current one");
    }
    // in case of status stays unchanged, then consider action as SUCCESS
    return ActionCallStatus::Success;
  } else {
    return ActionCallStatus::InvalidParam;
  }
}

template <class MTrait>
template <class Status, typename... Args, AllowOnlyStatusT<MTrait, Status>>
ActionCallStatus Stub<MTrait>::setStatus(Args &&... args) {
  return setStatus(std::make_shared<Status>(std::forward<Args>(args)...));
}

template <class MTrait>
template <class Status, AllowOnlyStatusT<MTrait, Status>>
std::shared_ptr<Status> Stub<MTrait>::getStatus() {
  if (auto baseStatus =
          provider_->getStatus(MTrait::template getOperationID<Status>())) {
    return MTrait::template decode<Status>(baseStatus);
  } else {
    return {};
  }
}

template <class MTrait>
template <class Attributes, AllowOnlyAttributesT<MTrait, Attributes>>
ActionCallStatus
Stub<MTrait>::broadcastSignal(const std::shared_ptr<Attributes> &attr) {
  return provider_->broadcastSignal(
      MTrait::template getOperationID<Attributes>(),
      MTrait::template encode(attr));
}

template <class MTrait>
template <class Attributes, typename... Args,
          AllowOnlyAttributesT<MTrait, Attributes>>
ActionCallStatus Stub<MTrait>::broadcastSignal(Args &&... args) {
  return broadcastSignal(
      std::make_shared<Attributes>(std::forward<Args>(args)...));
}

template <class MTrait>
template <class Signal, AllowOnlySignalT<MTrait, Signal>>
ActionCallStatus Stub<MTrait>::broadcastSignal() {
  return provider_->broadcastSignal(MTrait::template getOperationID<Signal>(),
                                    {});
}

template <class MTrait>
template <class RequestOrInput,
          AllowOnlyRequestOrInputT<MTrait, RequestOrInput>>
bool Stub<MTrait>::registerRequestHandler(
    RequestHandlerFunction<RequestOrInput> handlerFunction) {
  if (executor_) {
    auto requestHandler =
        [handlerFunction = std::move(handlerFunction),
         executor = this->executor_](
            const std::shared_ptr<RequestIF> &request) mutable {
          executor->execute(
              std::bind(std::move(handlerFunction),
                        std::shared_ptr<Request<RequestOrInput>>(
                            new Request<RequestOrInput>{request})));
        };

    return provider_->registerRequestHandler(
        MTrait::template getOperationID<RequestOrInput>(),
        std::move(requestHandler));
  } else {
    MAF_LOGGER_ERROR("Executer for Stub of service id `",
                     provider_->serviceID(),
                     "` has not been set yet(nullptr)!");
  }

  return false;
}

template <class MTrait>
bool Stub<MTrait>::unregisterRequestHandler(const OpID &opID) {
  return provider_->unregisterRequestHandler(opID);
}

template <class MTrait> void Stub<MTrait>::startServing() {
  provider_->startServing();
}

template <class MTrait> void Stub<MTrait>::stopServing() {
  provider_->stopServing();
}

template <class MTrait>
void Stub<MTrait>::setExecutor(Stub::ExecutorPtr executor) {
  if (executor) {
    executor_ = std::move(executor);
  }
}

template <class MTrait>
typename Stub<MTrait>::ExecutorPtr Stub<MTrait>::getExecutor() const {
  return executor_;
}

} // namespace messaging
} // namespace maf

#endif
