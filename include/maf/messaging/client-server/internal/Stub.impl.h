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

template <class PTrait>
Stub<PTrait>::Stub(ProviderPtr provider, ExecutorPtr executor)
    : provider_(std::move(provider)), executor_{std::move(executor)} {}

template <class PTrait>
typename Stub<PTrait>::StubPtr
Stub<PTrait>::createStub(const ConnectionType &contype, const Address &addr,
                         const ServiceID &sid, ExecutorPtr executor) {
  if (auto provider =
          CSManager::instance().getServiceProvider(contype, addr, sid)) {
    return std::shared_ptr<Stub>{
        new Stub(std::move(provider), std::move(executor))};
  }

  return {};
}

template <class PTrait> const ServiceID &Stub<PTrait>::serviceID() const {
  return provider_->serviceID();
}

template <class PTrait>
template <class Status, AllowOnlyStatusT<PTrait, Status>>
ActionCallStatus
Stub<PTrait>::setStatus(const std::shared_ptr<Status> &status) {
  if (status) {
    auto propertyID = PTrait::template getOperationID<Status>();

    auto oldStatus = provider_->getStatus(propertyID);
    auto newStatus = PTrait::template translate(status);

    if (!newStatus->equal(oldStatus.get())) {
      MAF_LOGGER_VERBOSE("Property `", propertyID, "`'s status changed: \n",
                         PTrait::template dump<Status>(status));
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

template <class PTrait>
template <class Status, typename... Args, AllowOnlyStatusT<PTrait, Status>>
ActionCallStatus Stub<PTrait>::setStatus(Args &&... args) {
  return setStatus(std::make_shared<Status>(std::forward<Args>(args)...));
}

template <class PTrait>
template <class Status, AllowOnlyStatusT<PTrait, Status>>
std::shared_ptr<Status> Stub<PTrait>::getStatus() {
  if (auto baseStatus =
          provider_->getStatus(PTrait::template getOperationID<Status>())) {
    return PTrait::template translate<Status>(baseStatus);
  } else {
    return {};
  }
}

template <class PTrait>
template <class Attributes, AllowOnlyAttributesT<PTrait, Attributes>>
ActionCallStatus
Stub<PTrait>::broadcastSignal(const std::shared_ptr<Attributes> &attr) {
  auto sigID = PTrait::template getOperationID<Attributes>();
  MAF_LOGGER_VERBOSE("Broadcast signal `", sigID, "`: \n",
                     PTrait::template dump<Attributes>(attr));
  return provider_->broadcastSignal(sigID, PTrait::template translate(attr));
}

template <class PTrait>
template <class Attributes, typename... Args,
          AllowOnlyAttributesT<PTrait, Attributes>>
ActionCallStatus Stub<PTrait>::broadcastSignal(Args &&... args) {
  return broadcastSignal(
      std::make_shared<Attributes>(std::forward<Args>(args)...));
}

template <class PTrait>
template <class Signal, AllowOnlySignalT<PTrait, Signal>>
ActionCallStatus Stub<PTrait>::broadcastSignal() {
  return provider_->broadcastSignal(PTrait::template getOperationID<Signal>(),
                                    {});
}

template <class PTrait>
template <class RequestOrInput,
          AllowOnlyRequestOrInputT<PTrait, RequestOrInput>>
bool Stub<PTrait>::registerRequestHandler(
    RequestHandlerFunction<RequestOrInput> handlerFunction) {

  if (executor_) {
    auto requestHandler =
        [handlerFunction = std::move(handlerFunction),
         executor = executor_](const std::shared_ptr<RequestIF> &request) {
          executor->execute([request, callback = std::move(handlerFunction)] {
            callback(Request<RequestOrInput>{std::move(request)});
          });
        };

    return provider_->registerRequestHandler(
        PTrait::template getOperationID<RequestOrInput>(),
        std::move(requestHandler));
  } else {
    MAF_LOGGER_ERROR("Executer for Stub of service id `",
                     provider_->serviceID(),
                     "` has not been set yet(nullptr)!");
  }

  return false;
}

template <class PTrait>
bool Stub<PTrait>::unregisterRequestHandler(const OpID &opID) {
  return provider_->unregisterRequestHandler(opID);
}

template <class PTrait>
template <typename Request, AllowOnlyRequestT<PTrait, Request>>
bool Stub<PTrait>::unregisterRequestHandler() {
  return unregisterRequestHandler(PTrait::template getOperationID<Request>());
}

template <class PTrait> void Stub<PTrait>::startServing() {
  provider_->startServing();
}

template <class PTrait> void Stub<PTrait>::stopServing() {
  provider_->stopServing();
}

template <class PTrait>
void Stub<PTrait>::setExecutor(Stub::ExecutorPtr executor) {
  if (executor) {
    executor_ = std::move(executor);
  }
}

template <class PTrait>
typename Stub<PTrait>::ExecutorPtr Stub<PTrait>::getExecutor() const {
  return executor_;
}

template <class PTrait>
std::shared_ptr<Stub<PTrait>> Stub<PTrait>::with(Stub::ExecutorPtr executor) {
  assert(executor && "Custom executor must not be null");
  if (executor) {
    return StubPtr{new Stub{this->provider_, std::move(executor)}};
  }
  return {};
}

} // namespace messaging
} // namespace maf

#endif
