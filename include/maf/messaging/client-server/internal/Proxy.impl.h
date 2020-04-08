#ifndef MAF_MESSAGING_CLIENT_SERVER_PROXY_IMPL_H
#define MAF_MESSAGING_CLIENT_SERVER_PROXY_IMPL_H

#ifndef MAF_MESSAGING_CLIENT_SERVER_PROXY_H
#include <maf/messaging/client-server/Proxy.h>
#endif

#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/CSManager.h>
#include <maf/messaging/client-server/MessageTraitBase.h>

namespace maf {
namespace messaging {

template <class MTrait>
std::shared_ptr<Proxy<MTrait>>
Proxy<MTrait>::createProxy(const ConnectionType &contype, const Address &addr,
                           const ServiceID &sid, ExecutorPtr executor,
                           SVStatusObsvWptr statusObsv) noexcept {
  if (auto requester =
          CSManager::instance().getServiceRequester(contype, addr, sid)) {
    auto proxy = std::shared_ptr<Proxy<MTrait>>{
        new Proxy<MTrait>(requester, std::move(executor))};

    requester->registerServiceStatusObserver(std::move(statusObsv));
    return proxy;
  } else {
    MAF_LOGGER_FATAL("Failed to get Client with connection type: ", contype,
                     " and address: ", addr.dump(-1));
  }
  return {};
}

template <class MTrait>
Proxy<MTrait>::Proxy(RequesterPtr requester, ExecutorPtr executor) noexcept
    : requester_{std::move(requester)}, executor_{std::move(executor)} {}

template <class MTrait> const ServiceID &Proxy<MTrait>::serviceID() const {
  return requester_->serviceID();
}

template <class MTrait> Availability Proxy<MTrait>::serviceStatus() const {
  return requester_->serviceStatus();
}

template <class MTrait>
void Proxy<MTrait>::registerServiceStatusObserver(SVStatusObsvWptr observer) {
  requester_->registerServiceStatusObserver(std::move(observer));
}

template <class MTrait>
void Proxy<MTrait>::unregisterServiceStatusObserver(
    const SVStatusObsvWptr &observer) {
  requester_->unregisterServiceStatusObserver(observer);
}

template <class MTrait>
template <class CSParam>
CSMessageContentHandlerCallback Proxy<MTrait>::createResponseMsgHandlerCallback(
    ResponseProcessingCallback<CSParam> callback) {
  if (callback) {
    if (executor_) {
      return [callback = std::move(callback), executor = this->executor_](
                 const CSMsgContentBasePtr &msgContent) {
        auto operationID = MTrait::template getOperationID<CSParam>();

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

template <class MTrait>
template <class CSParam>
CSMessageContentHandlerCallback Proxy<MTrait>::createUpdateMsgHandlerCallback(
    UpdateProcessingCallback<CSParam> callback) {
  if (callback) {
    if (executor_) {
      return [callback = std::move(callback), executor = this->executor_](
                 const CSMsgContentBasePtr &msgContent) {
        executor->execute([msgContent, callback = std::move(callback)] {
          auto output = getOutput<CSParam>(msgContent);
          callback(output);
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

template <class MTrait>
template <class CSParam>
typename Proxy<MTrait>::template ResponsePtr<CSParam>
Proxy<MTrait>::getResposne(const CSMsgContentBasePtr &msgContent) {
  if (!msgContent) {
    return std::make_shared<ResponseType<CSParam>>(std::shared_ptr<CSParam>{});
  }

  if (msgContent->type() == CSMessageContentBase::Type::Error) {
    auto err = std::static_pointer_cast<CSError>(msgContent);
    MAF_LOGGER_ERROR("Got response error of request `",
                     MTrait::template getOperationID<CSParam>(),
                     "`: ", err->dump());
    return std::make_shared<ResponseType<CSParam>>(std::move(err));
  }

  if constexpr (!MTrait::template encodable<CSParam>()) {
    return std::make_shared<ResponseType<CSParam>>(std::shared_ptr<CSParam>{});
  }

  auto output = getOutput<CSParam>(msgContent);

  MAF_LOGGER_INFO(MTrait::template getOperationID<CSParam>(), "'s output:\n",
                  MTrait::template dump<CSParam>(output));

  return std::make_shared<ResponseType<CSParam>>(std::move(output));
}

template <class MTrait>
template <class CSParam>
std::shared_ptr<CSParam>
Proxy<MTrait>::getOutput(const CSMsgContentBasePtr &msgContent) {
  if constexpr (!MTrait::template encodable<CSParam>()) {
    return {};
  } else if (msgContent &&
             msgContent->type() != CSMessageContentBase::Type::Error) {
    MessageTraitBase::CodecStatus decodeStatus;
    auto output = MTrait::template decode<CSParam>(msgContent, &decodeStatus);

    if (decodeStatus != MessageTraitBase::MalformInput) {
      MAF_LOGGER_INFO(MTrait::template getOperationID<CSParam>(),
                      "'s output:\n", MTrait::template dump<CSParam>(output));
      return output;
    } else {
      MAF_LOGGER_WARN("Failed to get status of [",
                      MTrait::template getOperationID<CSParam>(),
                      "] "
                      "from server");
    }
  }

  return {};
}
template <class MTrait>
template <class Status, AllowOnlyStatusT<MTrait, Status>>
RegID Proxy<MTrait>::registerStatus(UpdateProcessingCallback<Status> callback,
                                    ActionCallStatus *callStatus) {
  auto propertyID = MTrait::template getOperationID<Status>();
  if (callback) {
    return requester_->registerStatus(
        propertyID, createUpdateMsgHandlerCallback(std::move(callback)),
        callStatus);
  } else {
    MAF_LOGGER_ERROR("Registering status id[ ", propertyID,
                     "] "
                     "failed, Please provide non-empty callback");
  }
  return {};
}

template <class MTrait>
template <class Attributes, AllowOnlyAttributesT<MTrait, Attributes>>
RegID Proxy<MTrait>::registerSignal(
    UpdateProcessingCallback<Attributes> callback,
    ActionCallStatus *callStatus) {
  auto signalID = MTrait::template getOperationID<Attributes>();
  if (callback) {
    return requester_->registerSignal(
        signalID, createUpdateMsgHandlerCallback(std::move(callback)),
        callStatus);
  } else {
    MAF_LOGGER_ERROR("Registering signal id[, ", signalID,
                     "] failed, Please provide non-empty callback");
  }
  return {};
}

template <class MTrait>
template <class Signal, AllowOnlySignalT<MTrait, Signal>>
RegID Proxy<MTrait>::registerSignal(std::function<void()> callback,
                                    ActionCallStatus *callStatus) {
  auto signalID = MTrait::template getOperationID<Signal>();
  if (callback) {
    if (executor_) {
      auto asyncHandler = [signalID, callback = std::move(callback),
                           executor = this->executor_](const auto &) {
        executor->execute(std::move(callback));
      };

      return requester_->registerSignal(signalID, std::move(asyncHandler),
                                        callStatus);
    } else {
      MAF_LOGGER_ERROR("Executer for Proxy of service id `",
                       requester_->serviceID(),
                       "` has not been set yet(nullptr)!");
    }
  } else {
    MAF_LOGGER_ERROR("Registering signal id[, ", signalID,
                     "] failed, Please provide non-empty callback");
  }

  return {};
}

template <class MTrait>
ActionCallStatus Proxy<MTrait>::unregisterBroadcast(const RegID &regID) {
  return requester_->unregisterBroadcast(regID);
}

template <class MTrait>
ActionCallStatus Proxy<MTrait>::unregisterBroadcastAll(const OpID &propertyID) {
  return requester_->unregisterBroadcastAll(propertyID);
}

template <class MTrait>
template <class Status, AllowOnlyStatusT<MTrait, Status>>
std::shared_ptr<Status> Proxy<MTrait>::getStatus(ActionCallStatus *callStatus,
                                                 RequestTimeoutMs timeout) {
  auto propertyID = MTrait::template getOperationID<Status>();
  if (auto msgContent =
          requester_->getStatus(propertyID, callStatus, timeout)) {
    return getOutput<Status>(msgContent);
  } else {
    return {};
  }
}

template <class MTrait>
template <class Status, AllowOnlyStatusT<MTrait, Status>>
ActionCallStatus
Proxy<MTrait>::getStatus(UpdateProcessingCallback<Status> onStatusCallback) {
  return requester_->getStatus(
      MTrait::template getOperationID<Status>(),
      createUpdateMsgHandlerCallback(std::move(onStatusCallback)));
}

template <class MTrait>
template <class RequestOrOutput, class Input,
          AllowOnlyRequestOrOutputT<MTrait, RequestOrOutput>,
          AllowOnlyInputT<MTrait, Input>>
RegID Proxy<MTrait>::sendRequestAsync(
    const std::shared_ptr<Input> &input,
    ResponseProcessingCallback<RequestOrOutput> callback,
    ActionCallStatus *callStatus) {
  return requester_->sendRequestAsync(
      MTrait::template getOperationID<RequestOrOutput>(),
      MTrait::template encode(input),
      createResponseMsgHandlerCallback(std::move(callback)), callStatus);
}

template <class MTrait>
template <class RequestOrOutput,
          AllowOnlyRequestOrOutputT<MTrait, RequestOrOutput>>
RegID Proxy<MTrait>::sendRequestAsync(
    ResponseProcessingCallback<RequestOrOutput> callback,
    ActionCallStatus *callStatus) {
  return requester_->sendRequestAsync(
      MTrait::template getOperationID<RequestOrOutput>(), {},
      createResponseMsgHandlerCallback(std::move(callback)), callStatus);
}

template <class MTrait>
template <class RequestOrOutput, class Input,
          AllowOnlyRequestOrOutputT<MTrait, RequestOrOutput>,
          AllowOnlyInputT<MTrait, Input>>
typename Proxy<MTrait>::template ResponsePtr<RequestOrOutput>
Proxy<MTrait>::sendRequest(const std::shared_ptr<Input> &input,
                           ActionCallStatus *callStatus,
                           RequestTimeoutMs timeout) {
  return sendRequest<RequestOrOutput>(
      MTrait::template getOperationID<RequestOrOutput>(),
      MTrait::template encode(input), callStatus, timeout);
}

template <class MTrait>
template <class RequestOrOutput,
          AllowOnlyRequestOrOutputT<MTrait, RequestOrOutput>>
typename Proxy<MTrait>::template ResponsePtr<RequestOrOutput>
Proxy<MTrait>::sendRequest(ActionCallStatus *callStatus,
                           RequestTimeoutMs timeout) {
  return sendRequest<RequestOrOutput>(
      MTrait::template getOperationID<RequestOrOutput>(), {}, callStatus,
      timeout);
}

template <class MTrait>
template <class RequestOrOutput>
typename Proxy<MTrait>::template ResponsePtr<RequestOrOutput>
Proxy<MTrait>::sendRequest(OpID actionID,
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

template <class MTrait> void Proxy<MTrait>::setExecutor(ExecutorPtr executor) {
  executor_ = std::move(executor);
}

template <class MTrait>
typename Proxy<MTrait>::ExecutorPtr
Proxy<MTrait>::getExecutor() const noexcept {
  return executor_;
}

} // namespace messaging
} // namespace maf

#endif
