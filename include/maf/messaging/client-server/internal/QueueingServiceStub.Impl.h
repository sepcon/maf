#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICESTUB_IMPL_H
#define MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICESTUB_IMPL_H

#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICESTUB_H
#   include <maf/messaging/client-server/QueueingServiceStub.h>
#endif

#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/internal/cs_param.h>
#include <maf/messaging/client-server/CSManager.h>

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
    const ServiceID& sid
    )
{
    if(auto provider = CSManager::instance().getServiceProvider(
             contype,
             addr,
             sid
             ))
    {
        return std::shared_ptr<Stub>{ new Stub(std::move(provider)) };
    }

    return {};
}

template<class MessageTrait>
const ServiceID &QueueingServiceStub<MessageTrait>::serviceID() const
{
    return _provider->serviceID();
}

template <class MessageTrait> template<class property_status,
         std::enable_if_t<
             std::is_base_of_v<cs_status, property_status>,
             bool>>
ActionCallStatus QueueingServiceStub<MessageTrait>::setStatus(
    const std::shared_ptr<property_status> &status
    )
{
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
template<class property_status, typename...Args,
         std::enable_if_t<
             std::is_base_of_v<cs_status, property_status>,
             bool>>
ActionCallStatus QueueingServiceStub<MessageTrait>::setStatus(Args&&... args)
{
    return setStatus(
        std::make_shared<property_status>(std::forward<Args>(args)...)
        );
}


template <class MessageTrait>
template<class property_status,
         std::enable_if_t<
             std::is_base_of_v<cs_status, property_status>,
             bool>>
std::shared_ptr<const property_status>
QueueingServiceStub<MessageTrait>::getStatus()
{
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

template <class MessageTrait>
template<class signal_attributes,
         std::enable_if_t<
             std::is_base_of_v<cs_attributes, signal_attributes>,
             bool>>
ActionCallStatus QueueingServiceStub<MessageTrait>::broadcastSignal(
    const std::shared_ptr<signal_attributes>& attr
    )
{
    return _provider->broadcastSignal(
        MessageTrait::template getOperationID<signal_attributes>(),
        MessageTrait::template encode<cs_attributes>(attr)
        );
}

template <class MessageTrait>
template<class signal_attributes, typename... Args,
         std::enable_if_t<
             std::is_base_of_v<cs_attributes, signal_attributes>,
             bool>>
ActionCallStatus QueueingServiceStub<MessageTrait>::broadcastSignal(
    Args&&... args
    )
{

    return broadcastSignal(
        std::make_shared<signal_attributes>(std::forward<Args>(args)...)
        );
}

template <class MessageTrait>
template<class signal_class,
         std::enable_if_t<
             std::is_base_of_v<cs_signal, signal_class>,
             bool>>
ActionCallStatus QueueingServiceStub<MessageTrait>::broadcastSignal()
{
    return _provider->broadcastSignal(
        MessageTrait::template getOperationID<signal_class>(),
        {}
        );
}


template<class MessageTrait>
template <class RequestInput,
         std::enable_if_t<
             std::is_base_of_v<cs_input, RequestInput> ||
             std::is_base_of_v<cs_operation, RequestInput>,
             bool>>
bool QueueingServiceStub<MessageTrait>::registerRequestHandler(
    RequestHandlerFunction<RequestInput> handlerFunction
    )
{
    using TheRequestType = RequestType<RequestInput>;
    auto requestID = MessageTrait::template getOperationID<RequestInput>();
    ComponentRef compref;
    if(getHandlerComponent(compref))
    {
        auto requestHandler =
            [ compref = std::move(compref),
             handlerFunction = std::move(handlerFunction) ]
            ( const std::shared_ptr<RequestInterface>& request ) mutable {
                if(auto comp = compref.lock()) {
                    comp->template post<CallbackExcMsg>(
                        std::move(handlerFunction),
                        std::make_shared<TheRequestType>(request)
                        );
                }
                else {
                    Logger::warn(
                        "Component is not available for handling request"
                        );
                }
            };

        return _provider->registerRequestHandler(
            requestID,
            std::move(requestHandler)
            );
    }
    else
    {
        Logger::error(
            "Failed on registerRequestHandler for id "
            "[", requestID, "]"
            "\nPlease make sure to call registerRequestHandler method"
            " in context of a component!"
            );
    }
    return false;
}


template<class MessageTrait>
bool QueueingServiceStub<MessageTrait>::unregisterRequestHandler(
    const OpID& opID
    )
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
