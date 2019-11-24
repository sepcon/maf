#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICESTUB_IMPL_H
#define MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICESTUB_IMPL_H

#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICESTUB_H
#   include <maf/messaging/client-server/QueueingServiceStub.h>
#endif

#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/internal/cs_param.h>
#include <maf/messaging/client-server/ServiceProvider.h>

namespace maf {
namespace messaging {

using logging::Logger;

template<class MessageTrait>
QueueingServiceStub<MessageTrait>::QueueingServiceStub(
    std::shared_ptr<ServiceProviderInterface> provider
    ) : _provider(std::move(provider))
{
}

template<class MessageTrait>
typename QueueingServiceStub<MessageTrait>::StubPtr
QueueingServiceStub<MessageTrait>::createStub(
    const ConnectionType &contype,
    const Address &addr,
    ServiceID sid
    )
{
    if(auto server = ServerFactory::instance().getServer(contype, addr))
    {
        return std::shared_ptr<Stub>{
            new Stub(
                std::make_shared<ServiceProvider>(sid, std::move(server))
                )
        };
    }
    return {};
}

template <class MessageTrait> template<class property_status>
ActionCallStatus QueueingServiceStub<MessageTrait>::setStatus(
    const std::shared_ptr<property_status> &status
    )
{
    static_assert(
        std::is_base_of_v<cs_status, property_status>,
        "status must be type of cs_status"
        );

    if(status)
    {
        auto propertyID = MessageTrait::template getOperationID<property_status>();

        auto oldStatus = _provider->getStatus(propertyID);
        auto newStatus = MessageTrait::template encode<cs_status>(status);

        if(!newStatus->equal(oldStatus.get()))
        {
            return _provider->setStatus(propertyID, newStatus);
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
template<class property_status, typename...Args>
ActionCallStatus QueueingServiceStub<MessageTrait>::setStatus(Args&&... args)
{
    static_assert(
        std::is_base_of_v<cs_status, property_status>,
        "status class must be type of cs_status"
        );

    return setStatus(
        std::make_shared<property_status>(std::forward<Args>(args)...)
        );
}


template<class MessageTrait>
template <class request_input,
         std::enable_if_t<
             std::is_base_of_v<cs_input, request_input>, bool>
         >
bool QueueingServiceStub<MessageTrait>::registerRequestHandler(
std::function<void(
            RequestPtr,
            const std::shared_ptr<request_input>&
            )> handlerFunction
    )
{
    auto opID = MessageTrait::template getOperationID<request_input>();
    ComponentRef compref;
    if(getHandlerComponent(compref))
    {
        auto requestHandler =
            [compref = std::move(compref),
             handlerFunction = std::move(handlerFunction)
        ] ( const std::shared_ptr<RequestInterface>& request ) mutable {
                if(auto comp = compref.lock()) {
                    auto requestT = std::make_shared<RequestType>(request);
                    auto requestInput = requestT->template getInput<request_input>();
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

        return _provider->registerRequestHandler( opID, std::move(requestHandler) );
    }
    else
    {
        Logger::error("Failed on registerRequestHandler for id[", opID, "]"
                      "\nPlease make sure to call registerRequestHandler method"
                      " in context of a component!");
    }

    return false;
}

template<class MessageTrait>
template <class request_class,
         std::enable_if_t<
             std::is_base_of_v<cs_request, request_class> ||
             std::is_base_of_v<cs_property, request_class>, bool>
         >
bool QueueingServiceStub<MessageTrait>::registerRequestHandler(
        std::function<void(RequestPtr)> handlerFunction
        )
{
    auto requestID = MessageTrait::template getOperationID<request_class>();
    ComponentRef compref;
    if(getHandlerComponent(compref))
    {

        auto requestHandler =
            [compref = std::move(compref), handlerFunction = std::move(handlerFunction)]
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

        return _provider->registerRequestHandler(requestID, std::move(requestHandler));
    }
    else
    {
        Logger::error("Failed on registerRequestHandler for id[", requestID, "]"
                      "\nPlease make sure to call registerRequestHandler method"
                      " in context of a component!"
                      );
    }
    return false;
}

template<class MessageTrait>
bool QueueingServiceStub<MessageTrait>::unregisterRequestHandler(OpID opID)
{
    return _provider->unregisterRequestHandler(opID);
}

template<class MessageTrait>
void QueueingServiceStub<MessageTrait>::setMainComponent(ComponentRef copmref)
{
    _compref = std::move(copmref);
}

template<class MessageTrait>
void QueueingServiceStub<MessageTrait>::onComponentUnavailable()
{
    Logger::error(
        "The stub handler for service ID " ,
        serviceID() ,
        " has no longer existed, then unregister this Stub to server"
        );
}

template<class MessageTrait>
void QueueingServiceStub<MessageTrait>::startServing()
{
    _provider->startServing();
}

template<class MessageTrait>
void QueueingServiceStub<MessageTrait>::stopServing()
{
    _provider->stopServing();
}


template <class MessageTrait>
template<class property_status>
std::shared_ptr<const property_status> QueueingServiceStub<MessageTrait>::getStatus()
{
    static_assert(
        std::is_base_of_v<cs_status, property_status>,
        "status must be type of cs_status"
        );

    if(auto baseStatus = _provider->getStatus(
            MessageTrait::template getOperationID<property_status>()
            )
        )
    {
        return MessageTrait::template decode<const property_status>(baseStatus);
    }
    else
    {
        return {};
    }
}

template<class MessageTrait>
bool QueueingServiceStub<MessageTrait>::getHandlerComponent(
    ComponentRef& compref
    ) const
{
    bool success = false;
    if(auto component = Component::getActiveSharedPtr())
    {
        compref = component;
        success = true;
    }
    else if(_compref.lock())
    {
        compref = _compref;
        success = true;
    }

    return success;
}

} // messaging
} // maf

#endif
