#ifndef MAF_MESSAGING_CLIENT_SERVER_PROXY_IMPL_H
#define MAF_MESSAGING_CLIENT_SERVER_PROXY_IMPL_H

#ifndef MAF_MESSAGING_CLIENT_SERVER_PROXY_H
#include <maf/messaging/client-server/BasicProxy.h>
#endif

#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/CSManager.h>
#include <maf/messaging/client-server/ParamTraitBase.h>
#include <maf/utils/Pointers.h>

namespace maf {
namespace messaging {

template <class PTrait>
std::shared_ptr<BasicProxy<PTrait>> BasicProxy<PTrait>::createProxy(
    const ConnectionType &contype, const Address &addr, const ServiceID &sid,
    ExecutorPtr executor, SVStatusObsvWptr statusObsv) noexcept {
  if (auto requester =
          CSManager::instance().getServiceRequester(contype, addr, sid)) {

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
                               ExecutorPtr executor) noexcept
    : requester_{std::move(requester)}, executor_{std::move(executor)} {}

template <class PTrait> const ServiceID &BasicProxy<PTrait>::serviceID() const {
  return requester_->serviceID();
}

template <class PTrait> Availability BasicProxy<PTrait>::serviceStatus() const {
  return requester_->serviceStatus();
}

template <class PTrait>
void BasicProxy<PTrait>::registerServiceStatusObserver(
    SVStatusObsvWptr observer) {
  requester_->registerServiceStatusObserver(std::move(observer));
}

template <class PTrait>
void BasicProxy<PTrait>::unregisterServiceStatusObserver(
    const SVStatusObsvWptr &observer) {
  requester_->unregisterServiceStatusObserver(observer);
}

template <class PTrait>
template <class CSParam>
CSMessageContentHandlerCallback
BasicProxy<PTrait>::createResponseMsgHandlerCallback(
    ResponseProcessingCallback<CSParam> callback) {
  if (callback) {
    if (executor_) {
      return [callback = std::move(callback), executor = this->executor_](
                 const CSMsgContentBasePtr &msgContent) mutable {
        auto operationID = getOpID<CSParam>();

        executor->execute([msgContent, callback = std::move(callback)] {
          // getResposne must be called on thread of executor
          // try not block thread of service requester untill
          // finish translating message
          callback(getResposne<CSParam>(msgContent));
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
CSMessageContentHandlerCallback
BasicProxy<PTrait>::createUpdateMsgHandlerCallback(
    UpdateProcessingCallback<CSParam> callback) {
  if (callback) {
    if (executor_) {
      return [callback = std::move(callback), executor = this->executor_](
                 const CSMsgContentBasePtr &msgContent) mutable {
        executor->execute([msgContent, callback = std::move(callback)] {
          callback(getOutput<CSParam>(msgContent));
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
BasicProxy<PTrait>::getResposne(const CSMsgContentBasePtr &msgContent) {
  using ResponseType = Response<CSParam>;
  if (!msgContent) {
    return ResponseType{};
  }

  if (msgContent->type() == CSMessageContentBase::Type::Error) {
    auto err = std::static_pointer_cast<CSError>(msgContent);
    MAF_LOGGER_ERROR("Got response error of request `", getOpID<CSParam>(),
                     "`: ", err->dump());
    return ResponseType{std::move(err)};
  }

  if constexpr (!PTrait::template encodable<CSParam>()) {
    return ResponseType{};
  }

  return ResponseType{getOutput<CSParam>(msgContent)};
}

template <class PTrait>
template <class CSParam>
std::shared_ptr<CSParam>
BasicProxy<PTrait>::getOutput(const CSMsgContentBasePtr &msgContent) {
  if constexpr (!PTrait::template encodable<CSParam>()) {
    return {};
  } else if (msgContent &&
             msgContent->type() != CSMessageContentBase::Type::Error) {
    TranslationStatus decodeStatus;
    auto output = translate<CSParam>(msgContent, &decodeStatus);

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
    UpdateProcessingCallback<Status> callback, ActionCallStatus *callStatus) {
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
    UpdateProcessingCallback<Attributes> callback,
    ActionCallStatus *callStatus) {

  auto signalID = getOpID<Attributes>();
  if (auto translatedCallback =
          createUpdateMsgHandlerCallback(std::move(callback))) {
    return requester_->registerSignal(signalID, std::move(translatedCallback),
                                      callStatus);
  } else {
    util::assign_ptr(callStatus, ActionCallStatus::InvalidParam);
    MAF_LOGGER_ERROR("Failed to create translater callback for processing "
                     "signal `",
                     signalID, "`");
  }
  return {};
}

template <class PTrait>
template <class Signal, AllowOnlySignalT<PTrait, Signal>>
RegID BasicProxy<PTrait>::registerSignal(std::function<void()> callback,
                                         ActionCallStatus *callStatus) {
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
ActionCallStatus BasicProxy<PTrait>::unregister(const RegID &regID) {
  return requester_->unregister(regID);
}

template <class PTrait>
ActionCallStatus BasicProxy<PTrait>::unregisterAll(const OpID &propertyID) {
  return requester_->unregisterAll(propertyID);
}

template <class PTrait>
template <class Status, AllowOnlyStatusT<PTrait, Status>>
std::shared_ptr<Status>
BasicProxy<PTrait>::getStatus(ActionCallStatus *callStatus,
                              RequestTimeoutMs timeout) {
  auto propertyID = getOpID<Status>();
  if (auto msgContent =
          requester_->getStatus(propertyID, callStatus, timeout)) {
    return getOutput<Status>(msgContent);
  } else {
    return {};
  }
}

template <class PTrait>
template <class Status, AllowOnlyStatusT<PTrait, Status>>
ActionCallStatus BasicProxy<PTrait>::getStatus(
    UpdateProcessingCallback<Status> onStatusCallback) {
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
    ActionCallStatus *callStatus) {
  static_assert(getOpID<RequestOrOutput>() == getOpID<Input>(),
                "Input and Request/Output must have same OpID");
  return requester_->sendRequestAsync(
      getOpID<RequestOrOutput>(), translate(input),
      createResponseMsgHandlerCallback(std::move(callback)), callStatus);
}

template <class PTrait>
template <class RequestOrOutput,
          AllowOnlyRequestOrOutputT<PTrait, RequestOrOutput>>
RegID BasicProxy<PTrait>::sendRequestAsync(
    ResponseProcessingCallback<RequestOrOutput> callback,
    ActionCallStatus *callStatus) {
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
                                RequestTimeoutMs timeout) {
  static_assert(getOpID<RequestOrOutput>() == getOpID<Input>(),
                "Input and Output/Request must have same OpID");
  return sendRequest<RequestOrOutput>(getOpID<RequestOrOutput>(),
                                      translate(input), callStatus, timeout);
}

template <class PTrait>
template <class RequestOrOutput,
          AllowOnlyRequestOrOutputT<PTrait, RequestOrOutput>>
typename BasicProxy<PTrait>::template Response<RequestOrOutput>
BasicProxy<PTrait>::sendRequest(ActionCallStatus *callStatus,
                                RequestTimeoutMs timeout) {
  return sendRequest<RequestOrOutput>(getOpID<RequestOrOutput>(), {},
                                      callStatus, timeout);
}

template <class PTrait>
template <class RequestOrOutput>
typename BasicProxy<PTrait>::template Response<RequestOrOutput>
BasicProxy<PTrait>::sendRequest(const OpID &actionID,
                                const CSMsgContentBasePtr &requestInput,
                                ActionCallStatus *callStatus,
                                RequestTimeoutMs timeout) {
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
    return {};
  }
}

template <class PTrait>
void BasicProxy<PTrait>::setExecutor(ExecutorPtr executor) {
  executor_ = std::move(executor);
}

template <class PTrait>
typename BasicProxy<PTrait>::ExecutorPtr
BasicProxy<PTrait>::getExecutor() const noexcept {
  return executor_;
}

template <class PTrait>
std::shared_ptr<BasicProxy<PTrait>>
BasicProxy<PTrait>::with(BasicProxy::ExecutorPtr executor) {
  assert(executor && "custom executor must not be null");
  if (executor) {
    return std::shared_ptr<BasicProxy>(
        new BasicProxy{this->requester_, std::move(executor)});
  }
  return {};
}

} // namespace messaging
} // namespace maf

#endif
