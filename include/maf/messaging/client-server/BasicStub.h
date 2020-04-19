#ifndef MAF_MESSAGING_CLIENT_SERVER_STUB_H
#define MAF_MESSAGING_CLIENT_SERVER_STUB_H

#include <maf/logging/Logger.h>
#include <maf/messaging/CallbackExecutorIF.h>
#include <maf/messaging/client-server/ServiceProviderIF.h>
#include <maf/patterns/Patterns.h>

#include "CSParamConstrains.h"
#include "RequestT.h"

namespace maf {
namespace messaging {

using namespace paco;

template <class PTrait> class BasicStub : public pattern::Unasignable {
  using StubPtr = std::shared_ptr<BasicStub>;

public:
  template <typename InputType> using Request = RequestT<PTrait, InputType>;

  template <typename InputType>
  using RequestHandlerFunction = std::function<void(Request<InputType>)>;

  using ExecutorPtr = std::shared_ptr<CallbackExecutorIF>;
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

} // namespace messaging
} // namespace maf

#ifndef MAF_MESSAGING_CLIENT_SERVER_STUB_IMPL_H
#include "BasicStub.impl.h"
#endif

#endif
