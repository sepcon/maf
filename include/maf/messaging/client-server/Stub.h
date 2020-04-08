#ifndef MAF_MESSAGING_CLIENT_SERVER_STUB_H
#define MAF_MESSAGING_CLIENT_SERVER_STUB_H

#include <maf/logging/Logger.h>
#include <maf/messaging/CallbackExecutorIF.h>
#include <maf/messaging/client-server/ServiceProviderIF.h>

#include "CSParamConstrains.h"
#include "RequestT.h"

namespace maf {
namespace messaging {

using namespace paco;

template <class MTrait> class Stub {
  using StubPtr = std::shared_ptr<Stub>;

public:
  template <typename InputType> using Request = RequestT<MTrait, InputType>;

  template <typename InputType>
  using RequestPtr = std::shared_ptr<Request<InputType>>;

  template <typename InputType = std::nullptr_t>
  using RequestHandlerFunction =
      std::function<void(const RequestPtr<InputType> &)>;

  using ExecutorPtr = std::shared_ptr<CallbackExecutorIF>;
  using ProviderPtr = std::shared_ptr<ServiceProviderIF>;

  static StubPtr createStub(const ConnectionType &contype, const Address &addr,
                            const ServiceID &sid, ExecutorPtr executor = {});

  const ServiceID &serviceID() const;

  template <class Status, AllowOnlyStatusT<MTrait, Status> = true>
  ActionCallStatus setStatus(const std::shared_ptr<Status> &status);

  template <class Status, typename... Args,
            AllowOnlyStatusT<MTrait, Status> = true>
  ActionCallStatus setStatus(Args &&...);

  template <class Status, AllowOnlyStatusT<MTrait, Status> = true>
  std::shared_ptr<Status> getStatus();

  template <class Attributes, AllowOnlyAttributesT<MTrait, Attributes> = true>
  ActionCallStatus broadcastSignal(const std::shared_ptr<Attributes> &attr);

  template <class Attributes, typename... Args,
            AllowOnlyAttributesT<MTrait, Attributes> = true>
  ActionCallStatus broadcastSignal(Args &&... args);

  template <class Signal, AllowOnlySignalT<MTrait, Signal> = true>
  ActionCallStatus broadcastSignal();

  template <class RequestOrInput,
            AllowOnlyRequestOrInputT<MTrait, RequestOrInput> = true>
  bool registerRequestHandler(
      RequestHandlerFunction<RequestOrInput> handlerFunction);

  bool unregisterRequestHandler(const OpID &opID);

  void startServing();
  void stopServing();

  void setExecutor(ExecutorPtr executor);
  ExecutorPtr getExecutor() const;

private:
  Stub(ProviderPtr provider, ExecutorPtr executor);

  ProviderPtr provider_;
  ExecutorPtr executor_;
};

} // namespace messaging
} // namespace maf

#ifndef MAF_MESSAGING_CLIENT_SERVER_STUB_IMPL_H
#include "internal/Stub.impl.h"
#endif

#endif
