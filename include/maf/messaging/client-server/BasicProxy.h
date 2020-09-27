#pragma once

#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/CSMgmt.h>
#include <maf/messaging/client-server/ParamTranslatingStatus.h>
#include <maf/utils/ExecutorIF.h>
#include <maf/utils/Pointers.h>

#include <cassert>

#include "CSParamConstrains.h"
#include "ResponseT.h"
#include "ServiceStatusObserverDelegater.h"

namespace maf {
namespace messaging {

using namespace paco;

template <class PTrait>
class BasicProxy {
 public:
  template <class CSParam>
  using Response = ResponseT<CSParam>;

  template <class CSParam>
  using ResponseProcessingCallback = std::function<void(Response<CSParam>)>;

  template <class CSParam>
  using NotificationProcessingCallback =
      std::function<void(const std::shared_ptr<CSParam> &)>;

  using ServiceStatusChangedCallback =
      ServiceStatusObserverDelegater::DelegateCallback;

  using ExecutorIFPtr = util::ExecutorIFPtr;
  using ServiceStatusObserverPtr = std::shared_ptr<ServiceStatusObserverIF>;
  using RequesterPtr = std::shared_ptr<ServiceRequesterIF>;
  static std::shared_ptr<BasicProxy> createProxy(
      const ConnectionType &contype, const Address &addr, const ServiceID &sid,
      ExecutorIFPtr executor = {},
      ServiceStatusObserverPtr statusObsv = {}) noexcept;

  const ServiceID &serviceID() const noexcept;
  Availability serviceStatus() const noexcept;

  template <class Status, AllowOnlyStatusT<PTrait, Status> = true>
  RegID registerStatus(NotificationProcessingCallback<Status> callback,
                       ActionCallStatus *callStatus = nullptr) noexcept;

  template <class Attributes, AllowOnlyAttributesT<PTrait, Attributes> = true>
  RegID registerSignal(NotificationProcessingCallback<Attributes> callback,
                       ActionCallStatus *callStatus = nullptr) noexcept;

  template <class Signal, AllowOnlySignalT<PTrait, Signal> = true>
  RegID registerSignal(std::function<void()> callback,
                       ActionCallStatus *callStatus = nullptr) noexcept;

  ActionCallStatus unregister(const RegID &regID) noexcept;
  ActionCallStatus unregisterAll(const OpID &propertyID) noexcept;

  template <class Status, AllowOnlyStatusT<PTrait, Status> = true>
  std::shared_ptr<Status> getStatus(ActionCallStatus *callStatus = nullptr,
                                    RequestTimeoutMs timeout = RequestTimeoutMs{
                                        0}) noexcept;

  template <class Status, AllowOnlyStatusT<PTrait, Status> = true>
  ActionCallStatus getStatus(
      NotificationProcessingCallback<Status> onStatusCallback) noexcept;

  template <class RequestOrOutput, class Input,
            AllowOnlyRequestOrOutputT<PTrait, RequestOrOutput> = true,
            AllowOnlyInputT<PTrait, Input> = true>
  RegID sendRequestAsync(
      const std::shared_ptr<Input> &requestInput,
      ResponseProcessingCallback<RequestOrOutput> callback = {},
      ActionCallStatus *callStatus = nullptr) noexcept;

  template <class RequestOrOutput,
            AllowOnlyRequestOrOutputT<PTrait, RequestOrOutput> = true>
  RegID sendRequestAsync(
      ResponseProcessingCallback<RequestOrOutput> callback = {},
      ActionCallStatus *callStatus = nullptr) noexcept;

  template <class RequestOrOutput, class Input,
            AllowOnlyRequestOrOutputT<PTrait, RequestOrOutput> = true,
            AllowOnlyInputT<PTrait, Input> = true>
  Response<RequestOrOutput> sendRequest(
      const std::shared_ptr<Input> &requestInput,
      ActionCallStatus *callStatus = nullptr,
      RequestTimeoutMs timeout = RequestTimeoutMs{0}) noexcept;

  template <class RequestOrOutput,
            AllowOnlyRequestOrOutputT<PTrait, RequestOrOutput> = true>
  Response<RequestOrOutput> sendRequest(
      ActionCallStatus *callStatus = nullptr,
      RequestTimeoutMs timeout = RequestTimeoutMs{0}) noexcept;

  void abortRequest(const RegID &regID, ActionCallStatus *callStatus = nullptr);

  void registerServiceStatusObserver(
      ServiceStatusObserverPtr observer) noexcept;
  void unregisterServiceStatusObserver(
      const ServiceStatusObserverPtr &observer) noexcept;

  std::shared_ptr<ServiceStatusObserverIF> onServiceStatusChanged(
      ServiceStatusChangedCallback callback) noexcept;

  void setExecutor(ExecutorIFPtr executor) noexcept;
  ExecutorIFPtr getExecutor() const noexcept;
  std::shared_ptr<BasicProxy> with(ExecutorIFPtr executor) noexcept;
  RequesterPtr getRequester() const noexcept;

 private:
  BasicProxy(RequesterPtr requester, ExecutorIFPtr executor) noexcept;

  template <class CSParam>
  CSPayloadProcessCallback createUpdateMsgHandlerCallback(
      NotificationProcessingCallback<CSParam> callback) noexcept;

  template <class CSParam>
  CSPayloadProcessCallback createResponseMsgHandlerCallback(
      ResponseProcessingCallback<CSParam> callback) noexcept;

  template <class OperationOrOutput>
  Response<OperationOrOutput> sendRequest(const OpID &actionID,
                                          const CSPayloadIFPtr &requestInput,
                                          ActionCallStatus *callStatus,
                                          RequestTimeoutMs timeout) noexcept;

  template <class CSParam>
  static Response<CSParam> getResposne(const CSPayloadIFPtr &) noexcept;

  template <class CSParam>
  static Response<CSParam> makeError(const OpID &opid,
                                     ActionCallStatus callstatus) noexcept;

  template <class CSParam>
  static std::shared_ptr<CSParam> convert(const CSPayloadIFPtr &) noexcept;

  template <class T>
  static constexpr auto getOpID() noexcept {
    return PTrait::template getOperationID<T>();
  }

  template <class Message>
  static std::shared_ptr<Message> translate(
      const CSPayloadIFPtr &csMsgContent, TranslationStatus *status = nullptr) {
    return PTrait::template translate<Message>(csMsgContent, status);
  }
  template <class Message>
  static CSPayloadIFPtr translate(const std::shared_ptr<Message> &msg) {
    return PTrait::template translate(msg);
  }

  RequesterPtr requester_;
  ExecutorIFPtr executor_;
};

#ifndef MAF_NO_STATIC_OPERATION_ID
#define MAF_ASSERT_SAME_OPERATION_ID(Input, OutputOrRequest)         \
  static_assert(PTrait::template IsSameOpID<Input, OutputOrRequest>, \
                "Input and Request/Output must have same OpID");
#else
#define MAF_ASSERT_SAME_OPERATION_ID(Input, OutputOrRequest)
#endif

template <class PTrait>
std::shared_ptr<BasicProxy<PTrait>> BasicProxy<PTrait>::createProxy(
    const ConnectionType &contype, const Address &addr, const ServiceID &sid,
    ExecutorIFPtr executor, ServiceStatusObserverPtr statusObsv) noexcept {
  if (auto requester = csmgmt::getServiceRequester(contype, addr, sid)) {
    auto proxy = std::shared_ptr<BasicProxy<PTrait>>{
        new BasicProxy<PTrait>(std::move(requester), std::move(executor))};
    proxy->registerServiceStatusObserver(std::move(statusObsv));
    return proxy;

  } else {
    MAF_LOGGER_FATAL("Failed to get Client with connection type: ", contype,
                     " and address: ", addr.dump(-1));
  }
  return {};
}

template <class PTrait>
BasicProxy<PTrait>::BasicProxy(RequesterPtr requester,
                               ExecutorIFPtr executor) noexcept
    : requester_{std::move(requester)}, executor_{std::move(executor)} {}

template <class PTrait>
const ServiceID &BasicProxy<PTrait>::serviceID() const noexcept {
  return requester_->serviceID();
}

template <class PTrait>
Availability BasicProxy<PTrait>::serviceStatus() const noexcept {
  return requester_->serviceStatus();
}

template <class PTrait>
void BasicProxy<PTrait>::registerServiceStatusObserver(
    ServiceStatusObserverPtr observer) noexcept {
  requester_->registerServiceStatusObserver(std::move(observer));
}

template <class PTrait>
void BasicProxy<PTrait>::unregisterServiceStatusObserver(
    const ServiceStatusObserverPtr &observer) noexcept {
  requester_->unregisterServiceStatusObserver(observer);
}

template <class PTrait>
std::shared_ptr<ServiceStatusObserverIF>
BasicProxy<PTrait>::onServiceStatusChanged(
    ServiceStatusChangedCallback callback) noexcept {
  if (executor_ && callback) {
    // make shared might throw?
    auto observer = std::make_shared<ServiceStatusObserverDelegater>(
        executor_, std::move(callback));
    registerServiceStatusObserver(observer);
    return observer;
  }
  return {};
}

template <class PTrait>
template <class CSParam>
CSPayloadProcessCallback BasicProxy<PTrait>::createResponseMsgHandlerCallback(
    ResponseProcessingCallback<CSParam> callback) noexcept {
  if (callback) {
    if (executor_) {
      return [callback = std::move(callback), executor = this->executor_](
                 const CSPayloadIFPtr &payload) mutable {
        auto operationID = getOpID<CSParam>();

        executor->execute([payload, callback = std::move(callback)] {
          // getResposne must be called on thread of executor
          // try not block thread of service requester untill
          // finish translating message
          callback(getResposne<CSParam>(payload));
        });
      };
    } else {
      MAF_LOGGER_ERROR("Executer for Stub of service id `",
                       requester_->serviceID(),
                       "` has not been set yet(nullptr)!");
    }
  }
  return {};
}

template <class PTrait>
template <class CSParam>
CSPayloadProcessCallback BasicProxy<PTrait>::createUpdateMsgHandlerCallback(
    NotificationProcessingCallback<CSParam> callback) noexcept {
  if (callback) {
    if (executor_) {
      return [callback = std::move(callback), executor = this->executor_](
                 const CSPayloadIFPtr &payload) mutable {
        executor->execute([payload, callback = std::move(callback)] {
          callback(convert<CSParam>(payload));
        });
      };
    } else {
      MAF_LOGGER_ERROR("Executer for Stub of service id `",
                       requester_->serviceID(),
                       "` has not been set yet(nullptr)!");
    }
  }
  return {};
}

template <class PTrait>
template <class CSParam>
typename BasicProxy<PTrait>::template Response<CSParam>
BasicProxy<PTrait>::getResposne(const CSPayloadIFPtr &payload) noexcept {
  using ResponseType = Response<CSParam>;
  if (!payload) {
    return typename ResponseType::OutputPtr{};
  }

  if (payload->type() == CSPayloadType::Error) {
    auto err = std::static_pointer_cast<CSError>(payload);
    MAF_LOGGER_ERROR("Got response error of request `", getOpID<CSParam>(),
                     "`: ", err->dump());
    return ResponseType{std::move(err)};
  }

  if constexpr (!PTrait::template encodable<CSParam>()) {
    return ResponseType{};
  }

  return ResponseType{convert<CSParam>(payload)};
}

template <class PTrait>
template <class CSParam>
typename BasicProxy<PTrait>::template Response<CSParam>
BasicProxy<PTrait>::makeError(const OpID &opid,
                              ActionCallStatus callstatus) noexcept {
  std::ostringstream oss;
  oss << "Operation failed `" << opid << "` with call status: " << callstatus;
  return std::make_shared<CSError>(oss.str(), CSErrorCode::OperationFailed);
}

template <class PTrait>
template <class CSParam>
std::shared_ptr<CSParam> BasicProxy<PTrait>::convert(
    const CSPayloadIFPtr &payload) noexcept {
  if constexpr (!PTrait::template encodable<CSParam>()) {
    return {};
  } else if (payload && payload->type() != CSPayloadType::Error) {
    TranslationStatus decodeStatus;
    auto output = translate<CSParam>(payload, &decodeStatus);

    if (decodeStatus != TranslationStatus::DestSrcMismatch) {
      MAF_LOGGER_VERBOSE(getOpID<CSParam>(), "'s output:\n",
                         PTrait::template dump<CSParam>(output));
      return output;
    } else {
      MAF_LOGGER_WARN("Failed to decode response of [", getOpID<CSParam>(),
                      "] "
                      "from server");
    }
  }

  return {};
}
template <class PTrait>
template <class Status, AllowOnlyStatusT<PTrait, Status>>
RegID BasicProxy<PTrait>::registerStatus(
    NotificationProcessingCallback<Status> callback,
    ActionCallStatus *callStatus) noexcept {
  auto propertyID = getOpID<Status>();
  if (auto translatorCallback =
          createUpdateMsgHandlerCallback(std::move(callback))) {
    return requester_->registerStatus(propertyID, std::move(translatorCallback),
                                      callStatus);
  } else {
    util::assign_ptr(callStatus, ActionCallStatus::InvalidParam);
    MAF_LOGGER_ERROR("Registering status id[ ", propertyID,
                     "] "
                     "failed, Please provide non-empty callback");
  }
  return {};
}

template <class PTrait>
template <class Attributes, AllowOnlyAttributesT<PTrait, Attributes>>
RegID BasicProxy<PTrait>::registerSignal(
    NotificationProcessingCallback<Attributes> callback,
    ActionCallStatus *callStatus) noexcept {
  auto signalID = getOpID<Attributes>();
  if (auto translatedCallback =
          createUpdateMsgHandlerCallback(std::move(callback))) {
    return requester_->registerSignal(signalID, std::move(translatedCallback),
                                      callStatus);
  } else {
    util::assign_ptr(callStatus, ActionCallStatus::InvalidParam);
    MAF_LOGGER_ERROR(
        "Failed to create translater callback for processing "
        "signal `",
        signalID, "`");
  }
  return {};
}

template <class PTrait>
template <class Signal, AllowOnlySignalT<PTrait, Signal>>
RegID BasicProxy<PTrait>::registerSignal(
    std::function<void()> callback, ActionCallStatus *callStatus) noexcept {
  auto signalID = getOpID<Signal>();
  if (callback) {
    if (executor_) {
      auto asyncHandler = [signalID, callback = std::move(callback),
                           executor = this->executor_](const auto &) {
        executor->execute(std::move(callback));
      };

      return requester_->registerSignal(signalID, std::move(asyncHandler),
                                        callStatus);
    } else {
      MAF_LOGGER_ERROR("Executer for BasicProxy of service id `",
                       requester_->serviceID(),
                       "` has not been set yet(nullptr)!");
    }
  } else {
    MAF_LOGGER_ERROR("Registering signal id[, ", signalID,
                     "] failed, Please provide non-empty callback");
  }

  return {};
}

template <class PTrait>
ActionCallStatus BasicProxy<PTrait>::unregister(const RegID &regID) noexcept {
  return requester_->unregister(regID);
}

template <class PTrait>
ActionCallStatus BasicProxy<PTrait>::unregisterAll(
    const OpID &propertyID) noexcept {
  return requester_->unregisterAll(propertyID);
}

template <class PTrait>
void BasicProxy<PTrait>::abortRequest(const RegID &regID,
                                      ActionCallStatus *callStatus) {
  requester_->abortRequest(regID, callStatus);
}

template <class PTrait>
template <class Status, AllowOnlyStatusT<PTrait, Status>>
std::shared_ptr<Status> BasicProxy<PTrait>::getStatus(
    ActionCallStatus *callStatus, RequestTimeoutMs timeout) noexcept {
  auto propertyID = getOpID<Status>();
  if (auto payload = requester_->getStatus(propertyID, callStatus, timeout)) {
    return convert<Status>(payload);
  } else {
    return {};
  }
}

template <class PTrait>
template <class Status, AllowOnlyStatusT<PTrait, Status>>
ActionCallStatus BasicProxy<PTrait>::getStatus(
    NotificationProcessingCallback<Status> onStatusCallback) noexcept {
  if (auto translatorCallback =
          createUpdateMsgHandlerCallback(std::move(onStatusCallback))) {
    return requester_->getStatus(getOpID<Status>(),
                                 std::move(translatorCallback));
  } else {
    return ActionCallStatus::InvalidParam;
  }
}

template <class PTrait>
template <class RequestOrOutput, class Input,
          AllowOnlyRequestOrOutputT<PTrait, RequestOrOutput>,
          AllowOnlyInputT<PTrait, Input>>
RegID BasicProxy<PTrait>::sendRequestAsync(
    const std::shared_ptr<Input> &input,
    ResponseProcessingCallback<RequestOrOutput> callback,
    ActionCallStatus *callStatus) noexcept {
  MAF_ASSERT_SAME_OPERATION_ID(Input, RequestOrOutput);

  return requester_->sendRequestAsync(
      getOpID<RequestOrOutput>(), translate(input),
      createResponseMsgHandlerCallback(std::move(callback)), callStatus);
}

template <class PTrait>
template <class RequestOrOutput,
          AllowOnlyRequestOrOutputT<PTrait, RequestOrOutput>>
RegID BasicProxy<PTrait>::sendRequestAsync(
    ResponseProcessingCallback<RequestOrOutput> callback,
    ActionCallStatus *callStatus) noexcept {
  return requester_->sendRequestAsync(
      getOpID<RequestOrOutput>(), {},
      createResponseMsgHandlerCallback(std::move(callback)), callStatus);
}

template <class PTrait>
template <class RequestOrOutput, class Input,
          AllowOnlyRequestOrOutputT<PTrait, RequestOrOutput>,
          AllowOnlyInputT<PTrait, Input>>
typename BasicProxy<PTrait>::template Response<RequestOrOutput>
BasicProxy<PTrait>::sendRequest(const std::shared_ptr<Input> &input,
                                ActionCallStatus *callStatus,
                                RequestTimeoutMs timeout) noexcept {
  MAF_ASSERT_SAME_OPERATION_ID(Input, RequestOrOutput);
  return sendRequest<RequestOrOutput>(getOpID<RequestOrOutput>(),
                                      translate(input), callStatus, timeout);
}

template <class PTrait>
template <class RequestOrOutput,
          AllowOnlyRequestOrOutputT<PTrait, RequestOrOutput>>
typename BasicProxy<PTrait>::template Response<RequestOrOutput>
BasicProxy<PTrait>::sendRequest(ActionCallStatus *callStatus,
                                RequestTimeoutMs timeout) noexcept {
  return sendRequest<RequestOrOutput>(getOpID<RequestOrOutput>(), {},
                                      callStatus, timeout);
}

template <class PTrait>
template <class RequestOrOutput>
typename BasicProxy<PTrait>::template Response<RequestOrOutput>
BasicProxy<PTrait>::sendRequest(const OpID &actionID,
                                const CSPayloadIFPtr &requestInput,
                                ActionCallStatus *callStatus,
                                RequestTimeoutMs timeout) noexcept {
  auto cstt = ActionCallStatus::FailedUnknown;
  auto rawResponse =
      requester_->sendRequest(actionID, requestInput, &cstt, timeout);

  if (callStatus) {
    *callStatus = cstt;
  }

  if (cstt == ActionCallStatus::Success) {
    return getResposne<RequestOrOutput>(rawResponse);
  } else {
    MAF_LOGGER_ERROR("Failed to send sync-request `", actionID,
                     "` to server with call status: ", cstt);
    return makeError<RequestOrOutput>(actionID, cstt);
  }
}

template <class PTrait>
void BasicProxy<PTrait>::setExecutor(ExecutorIFPtr executor) noexcept {
  executor_ = std::move(executor);
}

template <class PTrait>
typename BasicProxy<PTrait>::ExecutorIFPtr BasicProxy<PTrait>::getExecutor()
    const noexcept {
  return executor_;
}

template <class PTrait>
std::shared_ptr<BasicProxy<PTrait>> BasicProxy<PTrait>::with(
    BasicProxy::ExecutorIFPtr executor) noexcept {
  assert(executor && "custom executor must not be null");
  if (executor) {
    return std::shared_ptr<BasicProxy>(
        new BasicProxy{this->requester_, std::move(executor)});
  }
  return {};
}

template <class PTrait>
typename BasicProxy<PTrait>::RequesterPtr BasicProxy<PTrait>::getRequester()
    const noexcept {
  return requester_;
}

}  // namespace messaging
}  // namespace maf
