#ifndef MAF_MESSAGING_CLIENT_SERVER_PROXY_H
#define MAF_MESSAGING_CLIENT_SERVER_PROXY_H

#include <maf/messaging/CallbackExecutorIF.h>

#include "CSParamConstrains.h"
#include "ResponseT.h"
#include "ServiceRequesterIF.h"
#include "ServiceStatusObserverIF.h"

namespace maf {
namespace messaging {

using namespace paco;
template <class MTrait> class Proxy {
public:
  template <class CSParam> using ResponseType = ResponseT<CSParam>;

  template <class CSParam>
  using ResponsePtr = std::shared_ptr<ResponseT<CSParam>>;

  template <class CSParam>
  using ResponseProcessingCallback =
      std::function<void(const ResponsePtr<CSParam> &)>;

  template <class CSParam>
  using UpdateProcessingCallback =
      std::function<void(const std::shared_ptr<CSParam> &)>;

  using ExecutorPtr = std::shared_ptr<CallbackExecutorIF>;
  using SVStatusObsvWptr = std::weak_ptr<ServiceStatusObserverIF>;
  using RequesterPtr = std::shared_ptr<ServiceRequesterIF>;

  static constexpr auto InfiniteTimeout = RequestTimeoutMs::max();

  static std::shared_ptr<Proxy>
  createProxy(const ConnectionType &contype, const Address &addr,
              const ServiceID &sid, ExecutorPtr executor = {},
              SVStatusObsvWptr statusObsv = {}) noexcept;

  const ServiceID &serviceID() const;
  Availability serviceStatus() const;

  template <class Status, AllowOnlyStatusT<MTrait, Status> = true>
  RegID registerStatus(UpdateProcessingCallback<Status> callback,
                       ActionCallStatus *callStatus = nullptr);

  template <class Attributes, AllowOnlyAttributesT<MTrait, Attributes> = true>
  RegID registerSignal(UpdateProcessingCallback<Attributes> callback,
                       ActionCallStatus *callStatus = nullptr);

  template <class Signal, AllowOnlySignalT<MTrait, Signal> = true>
  RegID registerSignal(std::function<void()> callback,
                       ActionCallStatus *callStatus = nullptr);

  ActionCallStatus unregisterBroadcast(const RegID &regID);
  ActionCallStatus unregisterBroadcastAll(const OpID &propertyID);

  template <class Status, AllowOnlyStatusT<MTrait, Status> = true>
  std::shared_ptr<Status> getStatus(ActionCallStatus *callStatus = nullptr,
                                    RequestTimeoutMs timeout = InfiniteTimeout);

  template <class Status, AllowOnlyStatusT<MTrait, Status> = true>
  ActionCallStatus getStatus(UpdateProcessingCallback<Status> onStatusCallback);

  template <class RequestOrOutput, class Input,
            AllowOnlyRequestOrOutputT<MTrait, RequestOrOutput> = true,
            AllowOnlyInputT<MTrait, Input> = true>
  RegID
  sendRequestAsync(const std::shared_ptr<Input> &requestInput,
                   ResponseProcessingCallback<RequestOrOutput> callback = {},
                   ActionCallStatus *callStatus = nullptr);

  template <class RequestOrOutput,
            AllowOnlyRequestOrOutputT<MTrait, RequestOrOutput> = true>
  RegID
  sendRequestAsync(ResponseProcessingCallback<RequestOrOutput> callback = {},
                   ActionCallStatus *callStatus = nullptr);

  template <class RequestOrOutput, class Input,
            AllowOnlyRequestOrOutputT<MTrait, RequestOrOutput> = true,
            AllowOnlyInputT<MTrait, Input> = true>
  ResponsePtr<RequestOrOutput>
  sendRequest(const std::shared_ptr<Input> &requestInput,
              ActionCallStatus *callStatus = nullptr,
              RequestTimeoutMs timeout = InfiniteTimeout);

  template <class RequestOrOutput,
            AllowOnlyRequestOrOutputT<MTrait, RequestOrOutput> = true>
  ResponsePtr<RequestOrOutput>
  sendRequest(ActionCallStatus *callStatus = nullptr,
              RequestTimeoutMs timeout = InfiniteTimeout);

  void registerServiceStatusObserver(SVStatusObsvWptr observer);
  void unregisterServiceStatusObserver(const SVStatusObsvWptr &observer);

  void setExecutor(ExecutorPtr executor);
  ExecutorPtr getExecutor() const noexcept;

private:
  Proxy(RequesterPtr requester, ExecutorPtr executor) noexcept;

  template <class CSParam>
  CSMessageContentHandlerCallback
  createUpdateMsgHandlerCallback(UpdateProcessingCallback<CSParam> callback);

  template <class CSParam>
  CSMessageContentHandlerCallback createResponseMsgHandlerCallback(
      ResponseProcessingCallback<CSParam> callback);

  template <class OperationOrOutput>
  ResponsePtr<OperationOrOutput>
  sendRequest(OpID actionID, const CSMsgContentBasePtr &requestInput,
              ActionCallStatus *callStatus, RequestTimeoutMs timeout);

  template <class CSParam>
  static ResponsePtr<CSParam> getResposne(const CSMsgContentBasePtr &);

  template <class CSParam>
  static std::shared_ptr<CSParam> getOutput(const CSMsgContentBasePtr &);

  RequesterPtr requester_;
  ExecutorPtr executor_;
};

} // namespace messaging
} // namespace maf

#ifndef MAF_MESSAGING_CLIENT_SERVER_PROXY_IMPL_H
#include "internal/Proxy.impl.h"
#endif

#endif
