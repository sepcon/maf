#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICESTUB_IMPL_H
#define MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICESTUB_IMPL_H

#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICESTUB_H
#   include <maf/messaging/client-server/QueueingServiceStub.h>
#endif

#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/internal/cs_param.h>

namespace maf {
namespace messaging {

using logging::Logger;

template<class MessageTrait>
typename QueueingServiceStub<MessageTrait>::StubPtr
QueueingServiceStub<MessageTrait>::createStub(
        const ConnectionType &contype,
        const Address &addr, ServiceID sid
        )
{
    if(auto server = ServerFactory::instance().getServer(contype, addr))
    {
        if(auto provider = server->getServiceProvider(sid))
        {
            return std::static_pointer_cast<Stub>(provider);
        }
        else
        {
            return std::shared_ptr<Stub>{new Stub(sid, server)};
        }
    }
    return {};
}

template <class MessageTrait> template<class Status>
ActionCallStatus QueueingServiceStub<MessageTrait>::setStatus(
    const std::shared_ptr<Status> &status
    )
{
    static_assert(
        std::is_base_of_v<cs_status, Status>,
        "status must be type of cs_status"
        );

    if(status)
    {
        auto propertyID = MessageTrait::template getOperationID<Status>();

        auto oldStatus = MyBase::getStatus(propertyID);
        auto newStatus = MessageTrait::template encode<cs_status>(status);

        if(!newStatus->equal(oldStatus.get()))
        {
            return MyBase::setStatus(propertyID, newStatus);
        }
        else
        {
            Logger::warn("Trying to set new status thats not different with current one");
        }
        // in case of status stays unchanged, then consider action as SUCCESS
        return ActionCallStatus::Success;
    }
    else
    {
        return ActionCallStatus::InvalidParam;
    }
}

template <class MessageTrait>
template<class Status, typename...Args>
ActionCallStatus QueueingServiceStub<MessageTrait>::setStatus(Args&&... args)
{
    static_assert(
        std::is_base_of_v<cs_status, Status>,
        "status class must be type of cs_status"
        );

    return setStatus(
        std::make_shared<Status>(std::forward<Args>(args)...)
        );
}


template<class MessageTrait> template <class RequestInput>
void QueueingServiceStub<MessageTrait>::setRequestHandler(
std::function<void(
            RequestPtr,
            const std::shared_ptr<RequestInput>&
            )> handlerFunction
    )
{
    static_assert(
        std::is_base_of_v<cs_request, RequestInput>,
        "RequestInput class must be type of class cs_request"
        );

    auto opID = MessageTrait::template getOperationID<RequestInput>();
    auto requestHandler =
        [compref = _compref, handlerFunction = std::move(handlerFunction)]
        ( const std::shared_ptr<RequestInterface>& request ) mutable {
            if(auto comp = compref.lock()) {
                auto requestT = std::make_shared<RequestType>(request);
                auto requestInput = requestT->template getRequestContent<RequestInput>();
                comp->template postMessage<CallbackExcMsg>(
                    std::move(handlerFunction),
                    std::move(requestT),
                    std::move(requestInput)
                    );
            }
            else {
                Logger::warn("Component is not available for handling request");
            }
        };

    MyBase::setRequestHandler( opID, std::move(requestHandler) );
}

template<class MessageTrait>
void QueueingServiceStub<MessageTrait>::setRequestHandler(
        OpID actionID,
        std::function<void(RequestPtr)> handlerFunction
        )
{
    auto requestHandler =
        [compref = _compref, handlerFunction = std::move(handlerFunction)]
        ( const std::shared_ptr<RequestInterface>& request ) mutable {
            if(auto comp = compref.lock()) {
                auto requestT = std::make_shared<RequestType>(request);
                comp->template postMessage<CallbackExcMsg>(
                    std::move(handlerFunction),
                    std::move(requestT)
                    );
            }
            else {
                Logger::warn("Component is not available for handling request");
            }
        };

    MyBase::setRequestHandler(actionID, std::move(requestHandler));
}

template<class MessageTrait>
void QueueingServiceStub<MessageTrait>::setMainComponent(ComponentRef copmref)
{
    _compref = std::move(copmref);
}

template<class MessageTrait>
QueueingServiceStub<MessageTrait>::QueueingServiceStub(
    ServiceID sid,
    std::weak_ptr<ServerInterface> server
    ) : ServiceStubDefault(sid, std::move(server)) {}

template<class MessageTrait>
void QueueingServiceStub<MessageTrait>::onClientAbortRequest(RequestAbortedCallback callback)
{
    if(auto comp = _compref.lock())
    {
        comp->template postMessage<CallbackExcMsg>(std::move(callback));
    }
    else
    {
        onComponentUnavailable();
    }
}

template<class MessageTrait>
void QueueingServiceStub<MessageTrait>::onComponentUnavailable()
{
    Logger::error("The stub handler for service ID " ,  serviceID() ,  " has no longer existed, then unregister this Stub to server");
}

template <class MessageTrait>
template<class Status>
std::shared_ptr<const Status> QueueingServiceStub<MessageTrait>::getStatus()
{
    static_assert(
        std::is_base_of_v<cs_status, Status>,
        "status must be type of cs_status"
        );

    if(auto baseStatus = MyBase::getStatus(
            MessageTrait::template getOperationID<Status>()
            )
        )
    {
        return MessageTrait::template decode<const Status>(baseStatus);
    }
    else
    {
        return {};
    }
}

} // messaging
} // maf

#endif
