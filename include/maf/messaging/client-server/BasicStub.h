#pragma once

#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/CSMgmt.h>
#include <maf/messaging/client-server/ParamTranslatingStatus.h>
#include <maf/messaging/client-server/ServiceProviderIF.h>
#include <maf/utils/ExecutorIF.h>

#include "CSParamConstrains.h"
#include "RequestT.h"

namespace maf {
namespace messaging {

using namespace paco;

template <class PTrait>
class BasicStub {
  using StubPtr = std::shared_ptr<BasicStub>;

 public:
  template <typename InputType>
  using Request = RequestT<PTrait, InputType>;

  template <typename InputType>
  using RequestHandlerFunction = std::function<void(Request<InputType>)>;

  template <typename StatusOrSignal>
  using OnNotificationCallback =
      std::function<void(std::shared_ptr<StatusOrSignal> const &)>;

  using OnEmptyNotificationCallback = std::function<void()>;

  using ExecutorIFPtr = util::ExecutorIFPtr;
  using ProviderPtr = std::shared_ptr<ServiceProviderIF>;

  static StubPtr createStub(const ConnectionType &contype, const Address &addr,
                            const ServiceID &sid, ExecutorIFPtr executor = {});

  const ServiceID &serviceID() const;
  Availability availability() const;

  template <class Status, AllowOnlyStatusT<PTrait, Status> = true>
  ActionCallStatus setStatus(const std::shared_ptr<Status> &status);

  template <class Status, typename... Args,
            AllowOnlyStatusT<PTrait, Status> = true>
  ActionCallStatus setStatus(Args &&...);

  template <class PropertyOrStatus,
            AllowOnlyPropertyOrStatusT<PTrait, PropertyOrStatus> = true>
  ActionCallStatus removeProperty(bool notify = false);

  ActionCallStatus removeProperty(const OpID &propID, bool notify = false);

  template <class Status, AllowOnlyStatusT<PTrait, Status> = true>
  std::shared_ptr<const Status> getStatus();

  template <class Status, AllowOnlyStatusT<PTrait, Status> = true>
  std::shared_ptr<Status> cloneStatus();

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

  template <class AttributesOrStatus,
            AllowOnlyAttributesOrStatusT<PTrait, AttributesOrStatus> = true>
  signal_slots::Connection registerNotification(
      OnNotificationCallback<AttributesOrStatus> callback);

  template <class Signal, AllowOnlySignalT<PTrait, Signal> = true>
  signal_slots::Connection registerNotification(
      OnEmptyNotificationCallback callback);

  void startServing();
  void stopServing();

  void setExecutor(ExecutorIFPtr executor);
  ExecutorIFPtr getExecutor() const;
  std::shared_ptr<BasicStub<PTrait>> with(ExecutorIFPtr executor);

 private:
  BasicStub(ProviderPtr provider, ExecutorIFPtr executor);

  template <class CSParam>
  CSPayloadProcessCallback createPayloadProcessCallback(
      OnNotificationCallback<CSParam> callback) noexcept;

  ProviderPtr provider_;
  ExecutorIFPtr executor_;
};

template <class PTrait>
BasicStub<PTrait>::BasicStub(ProviderPtr provider, ExecutorIFPtr executor)
    : provider_(std::move(provider)), executor_{std::move(executor)} {}

template <class PTrait>
typename BasicStub<PTrait>::StubPtr BasicStub<PTrait>::createStub(
    const ConnectionType &contype, const Address &addr, const ServiceID &sid,
    ExecutorIFPtr executor) {
  if (auto provider = csmgmt::getServiceProvider(contype, addr, sid)) {
    return std::shared_ptr<BasicStub>{
        new BasicStub(std::move(provider), std::move(executor))};
  }

  return {};
}

template <class PTrait>
const ServiceID &BasicStub<PTrait>::serviceID() const {
  return provider_->serviceID();
}

template <class PTrait>
Availability BasicStub<PTrait>::availability() const {
  return provider_->availability();
}

template <class PTrait>
template <class Status, AllowOnlyStatusT<PTrait, Status>>
ActionCallStatus BasicStub<PTrait>::setStatus(
    const std::shared_ptr<Status> &status) {
  return provider_->setStatus(PTrait::template getOperationID<Status>(),
                              PTrait::template translate(status));
}

template <class PTrait>
template <class Status, typename... Args, AllowOnlyStatusT<PTrait, Status>>
ActionCallStatus BasicStub<PTrait>::setStatus(Args &&... args) {
  return setStatus(std::make_shared<Status>(std::forward<Args>(args)...));
}

template <class PTrait>
template <class PropertyOrStatus,
          AllowOnlyPropertyOrStatusT<PTrait, PropertyOrStatus>>
ActionCallStatus BasicStub<PTrait>::removeProperty(bool notify) {
  return removeProperty(PTrait::template getOperationID<PropertyOrStatus>(),
                        notify);
}

template <class PTrait>
ActionCallStatus BasicStub<PTrait>::removeProperty(const OpID &propID,
                                                   bool notify) {
  return provider_->removeProperty(propID, notify);
}

template <class PTrait>
template <class Status, AllowOnlyStatusT<PTrait, Status>>
std::shared_ptr<const Status> BasicStub<PTrait>::getStatus() {
  if (auto baseStatus =
          provider_->getStatus(PTrait::template getOperationID<Status>())) {
    return PTrait::template translate<Status>(baseStatus);
  } else {
    return {};
  }
}

template <class PTrait>
template <class Status, AllowOnlyStatusT<PTrait, Status>>
std::shared_ptr<Status> BasicStub<PTrait>::cloneStatus() {
  static_assert(std::is_default_constructible_v<Status>,
                "must be default constructed");
  auto status = getStatus<const Status>();
  return std::make_shared<Status>(status ? *status : Status{});
}

template <class PTrait>
template <class Attributes, AllowOnlyAttributesT<PTrait, Attributes>>
ActionCallStatus BasicStub<PTrait>::broadcastSignal(
    const std::shared_ptr<Attributes> &attr) {
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

template <class PTrait>
template <class AttributesOrStatus,
          AllowOnlyAttributesOrStatusT<PTrait, AttributesOrStatus>>
signal_slots::Connection BasicStub<PTrait>::registerNotification(
    OnNotificationCallback<AttributesOrStatus> callback) {
  return provider_->registerNotification(
      PTrait::template getOperationID<AttributesOrStatus>(),
      createPayloadProcessCallback(std::move(callback)));
}

template <class PTrait>
template <class Signal, AllowOnlySignalT<PTrait, Signal>>
signal_slots::Connection BasicStub<PTrait>::registerNotification(
    OnEmptyNotificationCallback callback) {
  assert(executor_ && callback);
  return provider_->registerNotification(
      PTrait::template getOperationID<Signal>(),
      [callback = std::move(callback), executor = this->executor_](
          const CSPayloadIFPtr &) { executor->execute(callback); });
}

template <class PTrait>
template <class CSParam>
CSPayloadProcessCallback BasicStub<PTrait>::createPayloadProcessCallback(
    OnNotificationCallback<CSParam> callback) noexcept {
  assert(callback && executor_);
  return [callback = std::move(callback),
          executor = this->executor_](const CSPayloadIFPtr &payload) mutable {
    executor->execute([payload, callback] {
      // getResposne must be called on thread of executor
      // try not block thread of service requester untill
      // finish translating message
      auto transStatus = TranslationStatus::Success;
      auto translated =
          PTrait::template translate<CSParam>(payload, &transStatus);
      if (transStatus == TranslationStatus::Success) {
        callback(translated);
      } else {
        MAF_LOGGER_ERROR("Cannot translate message of type: ",
                         PTrait::template getOperationID<CSParam>());
      }
    });
  };
}

template <class PTrait>
void BasicStub<PTrait>::startServing() {
  provider_->startServing();
}

template <class PTrait>
void BasicStub<PTrait>::stopServing() {
  provider_->stopServing();
}

template <class PTrait>
void BasicStub<PTrait>::setExecutor(BasicStub::ExecutorIFPtr executor) {
  if (executor) {
    executor_ = std::move(executor);
  }
}

template <class PTrait>
typename BasicStub<PTrait>::ExecutorIFPtr BasicStub<PTrait>::getExecutor()
    const {
  return executor_;
}

template <class PTrait>
std::shared_ptr<BasicStub<PTrait>> BasicStub<PTrait>::with(
    BasicStub::ExecutorIFPtr executor) {
  assert(executor && "Custom executor must not be null");
  if (executor) {
    return StubPtr{new BasicStub{this->provider_, std::move(executor)}};
  }
  return {};
}

}  // namespace messaging
}  // namespace maf
