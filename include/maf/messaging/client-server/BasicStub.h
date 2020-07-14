#pragma once

#include "CSParamConstrains.h"
#include "RequestT.h"

#include <maf/logging/Logger.h>
#include <maf/messaging/ExecutorIF.h>
#include <maf/messaging/client-server/CSManager.h>
#include <maf/messaging/client-server/ServiceProviderIF.h>

namespace maf {
namespace messaging {

using namespace paco;

template <class PTrait> class BasicStub {
  using StubPtr = std::shared_ptr<BasicStub>;

public:
  template <typename InputType> using Request = RequestT<PTrait, InputType>;

  template <typename InputType>
  using RequestHandlerFunction = std::function<void(Request<InputType>)>;

  using ExecutorPtr = std::shared_ptr<ExecutorIF>;
  using ProviderPtr = std::shared_ptr<ServiceProviderIF>;

  static StubPtr createStub(const ConnectionType &contype, const Address &addr,
                            const ServiceID &sid, ExecutorPtr executor = {});

  const ServiceID &serviceID() const;

  template <class Status, AllowOnlyStatusT<PTrait, Status> = true>
  ActionCallStatus setStatus(const std::shared_ptr<Status> &status);

  template <class Status, typename... Args,
            AllowOnlyStatusT<PTrait, Status> = true>
  ActionCallStatus setStatus(Args &&...);

  template <class Status, AllowOnlyStatusT<PTrait, Status> = true>
  std::shared_ptr<Status> getStatus();

  template <class Attributes, AllowOnlyAttributesT<PTrait, Attributes> = true>
  ActionCallStatus broadcastSignal(const std::shared_ptr<Attributes> &attr);

  template <class Attributes, typename... Args,
            AllowOnlyAttributesT<PTrait, Attributes> = true>
  ActionCallStatus broadcastSignal(Args &&... args);

  template <class Signal, AllowOnlySignalT<PTrait, Signal> = true>
  ActionCallStatus broadcastSignal();

  template <class RequestOrInput,
            AllowOnlyRequestOrInputT<PTrait, RequestOrInput> = true>
  bool registerRequestHandler(
      RequestHandlerFunction<RequestOrInput> handlerFunction);

  bool unregisterRequestHandler(const OpID &opID);

  template <typename Request, AllowOnlyRequestT<PTrait, Request> = true>
  bool unregisterRequestHandler();

  void startServing();
  void stopServing();

  void setExecutor(ExecutorPtr executor);
  ExecutorPtr getExecutor() const;
  std::shared_ptr<BasicStub<PTrait>> with(ExecutorPtr executor);

private:
  BasicStub(ProviderPtr provider, ExecutorPtr executor);

  ProviderPtr provider_;
  ExecutorPtr executor_;
};

template <class PTrait>
BasicStub<PTrait>::BasicStub(ProviderPtr provider, ExecutorPtr executor)
    : provider_(std::move(provider)), executor_{std::move(executor)} {}

template <class PTrait>
typename BasicStub<PTrait>::StubPtr
BasicStub<PTrait>::createStub(const ConnectionType &contype,
                              const Address &addr, const ServiceID &sid,
                              ExecutorPtr executor) {
  if (auto provider = csmanagement::getServiceProvider(contype, addr, sid)) {
    return std::shared_ptr<BasicStub>{
        new BasicStub(std::move(provider), std::move(executor))};
  }

  return {};
}

template <class PTrait> const ServiceID &BasicStub<PTrait>::serviceID() const {
  return provider_->serviceID();
}

template <class PTrait>
template <class Status, AllowOnlyStatusT<PTrait, Status>>
ActionCallStatus
BasicStub<PTrait>::setStatus(const std::shared_ptr<Status> &status) {
  return provider_->setStatus(PTrait::template getOperationID<Status>(),
                              PTrait::template translate(status));
}

template <class PTrait>
template <class Status, typename... Args, AllowOnlyStatusT<PTrait, Status>>
ActionCallStatus BasicStub<PTrait>::setStatus(Args &&... args) {
  return setStatus(std::make_shared<Status>(std::forward<Args>(args)...));
}

template <class PTrait>
template <class Status, AllowOnlyStatusT<PTrait, Status>>
std::shared_ptr<Status> BasicStub<PTrait>::getStatus() {
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
BasicStub<PTrait>::broadcastSignal(const std::shared_ptr<Attributes> &attr) {
  auto sigID = PTrait::template getOperationID<Attributes>();
  MAF_LOGGER_VERBOSE("Broadcast signal `", sigID, "`: \n",
                     PTrait::template dump<Attributes>(attr));
  return provider_->broadcastSignal(sigID, PTrait::template translate(attr));
}

template <class PTrait>
template <class Attributes, typename... Args,
          AllowOnlyAttributesT<PTrait, Attributes>>
ActionCallStatus BasicStub<PTrait>::broadcastSignal(Args &&... args) {
  return broadcastSignal(
      std::make_shared<Attributes>(std::forward<Args>(args)...));
}

template <class PTrait>
template <class Signal, AllowOnlySignalT<PTrait, Signal>>
ActionCallStatus BasicStub<PTrait>::broadcastSignal() {
  return provider_->broadcastSignal(PTrait::template getOperationID<Signal>(),
                                    {});
}

template <class PTrait>
template <class RequestOrInput,
          AllowOnlyRequestOrInputT<PTrait, RequestOrInput>>
bool BasicStub<PTrait>::registerRequestHandler(
    RequestHandlerFunction<RequestOrInput> handlerFunction) {

  if (executor_) {
    auto requestHandler =
        [handlerFunction = std::move(handlerFunction), executor = executor_](
            const std::shared_ptr<RequestIF> &request) mutable {
          executor->execute([request = request,
                             callback = std::move(handlerFunction)]() mutable {
            callback(Request<RequestOrInput>{std::move(request)});
          });
        };

    return provider_->registerRequestHandler(
        PTrait::template getOperationID<RequestOrInput>(),
        std::move(requestHandler));
  } else {
    MAF_LOGGER_ERROR("Executer for BasicStub of service id `",
                     provider_->serviceID(),
                     "` has not been set yet(nullptr)!");
  }

  return false;
}

template <class PTrait>
bool BasicStub<PTrait>::unregisterRequestHandler(const OpID &opID) {
  return provider_->unregisterRequestHandler(opID);
}

template <class PTrait>
template <typename Request, AllowOnlyRequestT<PTrait, Request>>
bool BasicStub<PTrait>::unregisterRequestHandler() {
  return unregisterRequestHandler(PTrait::template getOperationID<Request>());
}

template <class PTrait> void BasicStub<PTrait>::startServing() {
  provider_->startServing();
}

template <class PTrait> void BasicStub<PTrait>::stopServing() {
  provider_->stopServing();
}

template <class PTrait>
void BasicStub<PTrait>::setExecutor(BasicStub::ExecutorPtr executor) {
  if (executor) {
    executor_ = std::move(executor);
  }
}

template <class PTrait>
typename BasicStub<PTrait>::ExecutorPtr BasicStub<PTrait>::getExecutor() const {
  return executor_;
}

template <class PTrait>
std::shared_ptr<BasicStub<PTrait>>
BasicStub<PTrait>::with(BasicStub::ExecutorPtr executor) {
  assert(executor && "Custom executor must not be null");
  if (executor) {
    return StubPtr{new BasicStub{this->provider_, std::move(executor)}};
  }
  return {};
}

} // namespace messaging
} // namespace maf
