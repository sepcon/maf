#ifndef MAF_MESSAGING_CLIENT_SERVER_PROXY_H
#define MAF_MESSAGING_CLIENT_SERVER_PROXY_H

#include "CSParamConstrains.h"
#include "ResponseT.h"
#include "ServiceRequesterIF.h"
#include "ServiceStatusObserverIF.h"
#include <maf/messaging/CallbackExecutorIF.h>
#include <maf/messaging/client-server/ParamTranslatingStatus.h>
#include <maf/patterns/Patterns.h>

namespace maf {
namespace messaging {

using namespace paco;

template <class PTrait> class BasicProxy : public pattern::Unasignable {
public:
  template <class CSParam> using Response = ResponseT<CSParam>;

  template <class CSParam>
  using ResponseProcessingCallback = std::function<void(Response<CSParam>)>;

  template <class CSParam>
  using UpdateProcessingCallback =
      std::function<void(const std::shared_ptr<CSParam> &)>;

  using ExecutorPtr = std::shared_ptr<CallbackExecutorIF>;
  using SVStatusObsvWptr = std::weak_ptr<ServiceStatusObserverIF>;
  using RequesterPtr = std::shared_ptr<ServiceRequesterIF>;

  static std::shared_ptr<BasicProxy>
  createProxy(const ConnectionType &contype, const Address &addr,
              const ServiceID &sid, ExecutorPtr executor = {},
              SVStatusObsvWptr statusObsv = {}) noexcept;

  const ServiceID &serviceID() const;
  Availability serviceStatus() const;

  template <class Status, AllowOnlyStatusT<PTrait, Status> = true>
  RegID registerStatus(UpdateProcessingCallback<Status> callback,
                       ActionCallStatus *callStatus = nullptr);

  template <class Attributes, AllowOnlyAttributesT<PTrait, Attributes> = true>
  RegID registerSignal(UpdateProcessingCallback<Attributes> callback,
                       ActionCallStatus *callStatus = nullptr);

  template <class Signal, AllowOnlySignalT<PTrait, Signal> = true>
  RegID registerSignal(std::function<void()> callback,
                       ActionCallStatus *callStatus = nullptr);

  ActionCallStatus unregister(const RegID &regID);
  ActionCallStatus unregisterAll(const OpID &propertyID);

  template <class Status, AllowOnlyStatusT<PTrait, Status> = true>
  std::shared_ptr<Status> getStatus(ActionCallStatus *callStatus = nullptr,
                                    RequestTimeoutMs timeout = InfiniteWait);

  template <class Status, AllowOnlyStatusT<PTrait, Status> = true>
  ActionCallStatus getStatus(UpdateProcessingCallback<Status> onStatusCallback);

  template <class RequestOrOutput, class Input,
            AllowOnlyRequestOrOutputT<PTrait, RequestOrOutput> = true,
            AllowOnlyInputT<PTrait, Input> = true>
  RegID
  sendRequestAsync(const std::shared_ptr<Input> &requestInput,
                   ResponseProcessingCallback<RequestOrOutput> callback = {},
                   ActionCallStatus *callStatus = nullptr);

  template <class RequestOrOutput,
            AllowOnlyRequestOrOutputT<PTrait, RequestOrOutput> = true>
  RegID
  sendRequestAsync(ResponseProcessingCallback<RequestOrOutput> callback = {},
                   ActionCallStatus *callStatus = nullptr);

  template <class RequestOrOutput, class Input,
            AllowOnlyRequestOrOutputT<PTrait, RequestOrOutput> = true,
            AllowOnlyInputT<PTrait, Input> = true>
  Response<RequestOrOutput>
  sendRequest(const std::shared_ptr<Input> &requestInput,
              ActionCallStatus *callStatus = nullptr,
              RequestTimeoutMs timeout = InfiniteWait);

  template <class RequestOrOutput,
            AllowOnlyRequestOrOutputT<PTrait, RequestOrOutput> = true>
  Response<RequestOrOutput>
  sendRequest(ActionCallStatus *callStatus = nullptr,
              RequestTimeoutMs timeout = InfiniteWait);

  void registerServiceStatusObserver(SVStatusObsvWptr observer);
  void unregisterServiceStatusObserver(const SVStatusObsvWptr &observer);

  void setExecutor(ExecutorPtr executor);
  ExecutorPtr getExecutor() const noexcept;
  std::shared_ptr<BasicProxy> with(ExecutorPtr executor);

private:
  BasicProxy(RequesterPtr requester, ExecutorPtr executor) noexcept;

  template <class CSParam>
  CSMessageContentHandlerCallback
  createUpdateMsgHandlerCallback(UpdateProcessingCallback<CSParam> callback);

  template <class CSParam>
  CSMessageContentHandlerCallback createResponseMsgHandlerCallback(
      ResponseProcessingCallback<CSParam> callback);

  template <class OperationOrOutput>
  Response<OperationOrOutput>
  sendRequest(const OpID &actionID, const CSMsgContentBasePtr &requestInput,
              ActionCallStatus *callStatus, RequestTimeoutMs timeout);

  template <class CSParam>
  static Response<CSParam> getResposne(const CSMsgContentBasePtr &);

  template <class CSParam>
  static std::shared_ptr<CSParam> getOutput(const CSMsgContentBasePtr &);

  template <class T> static constexpr auto getOpID() {
    return PTrait::template getOperationID<T>();
  }

  template <class Message>
  static std::shared_ptr<Message>
  translate(const CSMsgContentBasePtr &csMsgContent,
            TranslationStatus *status = nullptr) {
    return PTrait::template translate<Message>(csMsgContent, status);
  }
  template <class Message>
  static CSMsgContentBasePtr translate(const std::shared_ptr<Message> &msg) {
    return PTrait::template translate(msg);
  }

  RequesterPtr requester_;
  ExecutorPtr executor_;
};

} // namespace messaging
} // namespace maf

#ifndef MAF_MESSAGING_CLIENT_SERVER_PROXY_IMPL_H
#include "BasicProxy.impl.h"
#endif

#endif
