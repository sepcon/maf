#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICESTUB_H
#define MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICESTUB_H

#include <maf/messaging/client-server/ServiceStubDefault.h>
#include <maf/messaging/client-server/ServerFactory.h>
#include <maf/messaging/client-server/RequestT.h>
#include <maf/messaging/Component.h>
#include <maf/messaging/BasicMessages.h>
#include <maf/logging/Logger.h>

namespace maf {
namespace messaging {

template<class MessageTrait>
struct ClientRequestMessage : public CompMessageBase
{
    using RequestType = RequestT<MessageTrait>;
    using RequestPtr  = std::shared_ptr<RequestType>;
public:
    ClientRequestMessage(RequestPtr clp): _request(std::move(clp)){}
    RequestPtr getRequest() { return _request; }

    template<class CSMessageContentSpecific>
    std::shared_ptr<CSMessageContentSpecific> getRequestContent() const noexcept
    {
        return _request->template getRequestContent<CSMessageContentSpecific>();
    }

private:
    RequestPtr _request;
};

/**
 * @brief QueueingServiceStub class provides a generic interface of a ServiceProvider that is tight coupling with
 * a messaging::Component, to help handling Service Message in a queueing maner to prevent issue of data races
 * coping with multithreading application.
 * @class MessageTrait: must provide interfaces of translating specific type messages to CSMessage and vice versa
 * @class ControllingServer: must satisfy be a ServerInterface and is a pattern::SingletonObject (see patterns.h)
 */
template <class MessageTrait>
class QueueingServiceStub final : public ServiceStubDefault
{
    using Stub                      = QueueingServiceStub<MessageTrait>;
    using StubPtr                   = std::shared_ptr<Stub>;
    using MyBase                   = ServiceStubDefault;
public:
    using RequestType               = RequestT<MessageTrait>;
    using RequestPtr                = std::shared_ptr<RequestType>;
    using RequestMessageType        = ClientRequestMessage<MessageTrait>;
    using RequestMessagePtr         = std::shared_ptr<RequestMessageType>;

    static StubPtr createStub(const ConnectionType& contype, const Address& addr, ServiceID sid);

    template<class Status>
    ActionCallStatus setStatus(const std::shared_ptr<Status>& status);

    template<class Status, typename... Args>
    ActionCallStatus setStatus(Args&&...);

    template<class Status>
    std::shared_ptr<const Status> getStatus();

    template <class RequestInput>
    void setRequestHandler(
            std::function<void(RequestPtr,const std::shared_ptr<RequestInput>&)>
            handlerFunction
            );

    void setRequestHandler(
            OpID actionID,
            std::function<void(RequestPtr)> handlerFunction
            );

    void setMainComponent(ComponentRef copmref);
private:
    QueueingServiceStub(ServiceID sid, std::weak_ptr<ServerInterface> server);
    void onClientAbortRequest(RequestAbortedCallback callback) override;
    void onComponentUnavailable();

    ComponentRef _compref;
};


}
}

#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICESTUB_IMPL_H
#   include "internal/QueueingServiceStub.Impl.h"
#endif

#endif
