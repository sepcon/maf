#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICEPROXY_IMPL_H
#define MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICEPROXY_IMPL_H

#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICEPROXY_H
#   include <maf/messaging/client-server/QueueingServiceProxy.h>
#endif

#include <maf/messaging/client-server/MessageTraitBase.h>
#include <maf/messaging/client-server/ServiceStatusMsg.h>
#include <maf/messaging/client-server/CSManager.h>
#include <maf/messaging/BasicMessages.h>
#include <maf/logging/Logger.h>

namespace maf { using logging::Logger;
namespace messaging {


template<class MessageTrait>
std::shared_ptr<QueueingServiceProxy<MessageTrait>>
QueueingServiceProxy<MessageTrait>::createProxy(
    const ConnectionType &contype,
    const Address &addr,
    const ServiceID& sid
    )
{
    if(auto requester = CSManager::instance().getServiceRequester(
            contype,
            addr,
            sid))
    {
        auto proxy = std::shared_ptr<QueueingServiceProxy<MessageTrait>> {
                new QueueingServiceProxy<MessageTrait>(requester)
            };
        requester->addServiceStatusObserver(proxy);
        return proxy;
    }
    else
    {
        Logger::fatal(
            "Failed to get Client with connection type: ",
            contype,
            " and address: ",
            addr.dump(-1)
            );
    }
    return {};
}

template<class MessageTrait>
QueueingServiceProxy<MessageTrait>::QueueingServiceProxy(
        std::shared_ptr<ServiceRequesterInterface> requester
        ) : _requester{std::move(requester)} {}

template<class MessageTrait>
const ServiceID& QueueingServiceProxy<MessageTrait>::serviceID() const
{
    return _requester->serviceID();
}

template<class MessageTrait>
void QueueingServiceProxy<MessageTrait>::setMainComponent(ComponentRef compref)
{
    _compref = std::move(compref);
    if(_requester->serviceStatus() == Availability::Available)
    {
        updateServiceStatusToComponent(
                    Availability::Unavailable,
                    Availability::Available
                    );
    }

}

template<class MessageTrait>
void
QueueingServiceProxy<MessageTrait>::onServerStatusChanged(
    Availability oldStatus,
    Availability newStatus
    )
{
    if(newStatus != Availability::Available)
    { // Notify observers that service is not available as well
        onServiceStatusChanged(_requester->serviceID(), oldStatus, newStatus);
    }
}

template<class MessageTrait>
bool
QueueingServiceProxy<MessageTrait>::updateServiceStatusToComponent(
        Availability oldStatus,
        Availability newStatus
        )
{
    if(auto component = _compref.lock())
    {
        component->post<ServiceStatusMsg>(
                _requester->serviceID(),
                oldStatus,
                newStatus
            );
        return true;
    }
    return false;
}

template<class MessageTrait>
void
QueueingServiceProxy<MessageTrait>::onServiceStatusChanged(
    const ServiceID& /*sid*/,
    Availability oldStatus,
    Availability newStatus
    )
{
    updateServiceStatusToComponent(oldStatus, newStatus);
}


template<class MessageTrait>
template<class CSParam>
CSMessageContentHandlerCallback
QueueingServiceProxy<MessageTrait>::createResponseMsgHandlerCallback(
    ResponseProcessingCallback<CSParam> callback
    )
{
    if(!callback)
    {
        return {};
    }

    auto csContentHandlerCallback = [ callback = std::move(callback),
                                     compref = Component::getActiveWeakPtr() ]
        (const CSMsgContentBasePtr& msgContent)
    {
        auto operationID = MessageTrait::template getOperationID<CSParam>();

        if(auto component = compref.lock())
        {
            component->post<CallbackExcMsg>(
                callback,
                getResposne<CSParam>(msgContent)
                );
        }
        else
        {
            Logger::warn (
                "The component that sending the request of "
                "operationID: [", operationID, "] has no longer"
                " existed"
                );
        }

    };
    return csContentHandlerCallback;
}


template<class MessageTrait>
template<class CSParam>
CSMessageContentHandlerCallback
QueueingServiceProxy<MessageTrait>::createUpdateMsgHandlerCallback(
    UpdateProcessingCallback<CSParam> callback
    )
{
    if(!callback)
        return {}; //don't care about result

    // NOTE: this is not main component
    // Every component that emits the request should handle the request
    // itsef, then here we try to use Component::getActiveWeakPtr() instead
    // of _compref
    CSMessageContentHandlerCallback csContentHandlerCallback =
        [
            callback = std::move(callback),
            compref = Component::getActiveWeakPtr()
    ] (const CSMsgContentBasePtr& msgContent)
    {
        if(auto component = compref.lock())
        {
            // Request the requesting Component to execute the callback but
            component->post<CallbackExcMsg>(
                callback,
                getOutput<CSParam>(msgContent)
                );
        }
        else
        {
            Logger::warn (
                "The component that sending the request of "
                "operationID: [",
                MessageTrait::template getOperationID<CSParam>(), "] "
                "has no longer existed"
                );
        }
    };
    return csContentHandlerCallback;
}


template <class MessageTrait>
template<class CSParam>
typename QueueingServiceProxy<MessageTrait>::template ResponsePtrType<CSParam>
QueueingServiceProxy<MessageTrait>::getResposne(
    const CSMsgContentBasePtr& msgContent
    )
{
    if(!msgContent)
    {
        return std::make_shared<ResponseType<CSParam>>(
            std::shared_ptr<CSParam>{} );
    }

    if(msgContent->type() == CSMessageContentBase::Type::Error)
    {
        return std::make_shared<ResponseType<CSParam>>(
            std::static_pointer_cast<CSContentError>(msgContent)
            );
    }

    if constexpr(!MessageTrait::template encodable<CSParam>())
    {
        return std::make_shared<ResponseType<CSParam>>(
            std::shared_ptr<CSParam>{}
            );
    }

    return std::make_shared<ResponseType<CSParam>>(
        getOutput<CSParam>(msgContent)
        );

}

template <class MessageTrait> template<class CSParam>
std::shared_ptr<CSParam> QueueingServiceProxy<MessageTrait>::getOutput(
    const CSMsgContentBasePtr& msgContent
    )
{
    if constexpr ( !MessageTrait::template encodable<CSParam>() )
    {
        return {};
    }
    else if(msgContent->type() != CSMessageContentBase::Type::Error)
    {
        MessageTraitBase::CodecStatus decodeStatus;
        auto output = MessageTrait::template decode<CSParam>(
            msgContent,
            &decodeStatus
            );

        if(decodeStatus != MessageTraitBase::MalformInput)
        {
            return output;
        }
        else
        {
            Logger::warn(
                "Failed to get status of [",
                MessageTrait::template getOperationID<CSParam>(), "] "
                "from server"
                );
        }
    }

    return {};
}
template<class MessageTrait> template<class PropertyStatus,
         std::enable_if_t<
             std::is_base_of_v<cs_status, PropertyStatus>,
             bool>>
RegID QueueingServiceProxy<MessageTrait>::registerStatus(
    UpdateProcessingCallback<PropertyStatus> callback,
    ActionCallStatus* callStatus
    )
{
    auto propertyID = MessageTrait::template getOperationID<PropertyStatus>();
    if(callback)
    {
        return _requester->registerStatus(
            propertyID,
            createUpdateMsgHandlerCallback(std::move(callback)),
            callStatus
            );
    }
    else
    {
        Logger::error(
            "Registering status id[ ", propertyID, "] "
            "failed, Please provide non-empty callback"
            );
    }
    return {};
}

template<class MessageTrait> template<class SignalAttributes,
         std::enable_if_t<
             std::is_base_of_v<cs_attributes, SignalAttributes>,
             bool>>
RegID QueueingServiceProxy<MessageTrait>::registerSignal(
    UpdateProcessingCallback<SignalAttributes> callback,
    ActionCallStatus* callStatus
    )
{
    static_assert (
        std::is_base_of_v<cs_attributes, SignalAttributes>,
        "SignalAttributes must be a cs_attributes "
        );
    auto signalID = MessageTrait::template getOperationID<SignalAttributes>();
    if(callback)
    {
        return _requester->registerSignal(
            signalID,
            createUpdateMsgHandlerCallback(std::move(callback)),
            callStatus
            );
    }
    else
    {
        Logger::error("Registering signal id[, ",
                      signalID,
                      "] failed, Please provide non-empty callback");
    }
    return {};
}

template<class MessageTrait> template<class SignalClass,
         std::enable_if_t<
             std::is_base_of_v<cs_signal, SignalClass>,
             bool>>
RegID QueueingServiceProxy<MessageTrait>::registerSignal(
    std::function<void()> callback,
    ActionCallStatus* callStatus
    )
{
    static_assert (
        std::is_base_of_v<cs_signal, SignalClass>,
        "SignalClass must be a cs_signal "
        );

    auto signalID = MessageTrait::template getOperationID<SignalClass>();
    if(callback)
    {
        auto asyncHandler = [ signalID, callback = std::move(callback),
                             compref = Component::getActiveWeakPtr()
        ] (const auto&) mutable {
            if(auto component = compref.lock())
            {
                // Request the requesting Component to execute the callback but
                component->post<CallbackExcMsg>(std::move(callback));
            }
            else
            {
                Logger::warn(
                    "The component that sending the request of operationID: "
                    "[", signalID, "] has no longer existed"
                    );
            }
        };

        return _requester->registerSignal(
            signalID,
            std::move(asyncHandler),
            callStatus
            );
    }
    else
    {
        Logger::error("Registering signal id[, ",
                      signalID,
                      "] failed, Please provide non-empty callback");
    }
    return {};
}


template<class MessageTrait>
ActionCallStatus QueueingServiceProxy<MessageTrait>::unregisterBroadcast(const RegID& regID)
{
    return _requester->unregisterBroadcast(regID);
}

template<class MessageTrait>
ActionCallStatus QueueingServiceProxy<MessageTrait>::unregisterBroadcastAll(
    const OpID& propertyID
    )
{
    return _requester->unregisterBroadcastAll(propertyID);
}

template<class MessageTrait>
template<class PropertyStatus,
         std::enable_if_t<
             std::is_base_of_v<cs_status, PropertyStatus>,
             bool>>
std::shared_ptr<PropertyStatus>
QueueingServiceProxy<MessageTrait>::getStatus(
    RequestTimeoutMs timeout,
    ActionCallStatus* callStatus
    )
{
    auto propertyID = MessageTrait::template getOperationID<PropertyStatus>();
    if(auto msgContent =
            _requester->getStatus(
                propertyID,
                timeout,
                callStatus
                )
        )
    {
        return getOutput<PropertyStatus>(msgContent);
    }
    else
    {
        return {};
    }
}

template<class MessageTrait>
template <class OperationOrOutput,
         std::enable_if_t<
             std::is_base_of_v<cs_output, OperationOrOutput> ||
                 std::is_base_of_v<cs_operation, OperationOrOutput>, bool>>
RegID QueueingServiceProxy<MessageTrait>::sendRequestAsync(
    const std::shared_ptr<cs_input> &requestInput,
    ResponseProcessingCallback<OperationOrOutput> callback,
    ActionCallStatus* callStatus
    )
{
    return _requester->sendRequestAsync(
        MessageTrait::template getOperationID<OperationOrOutput>(),
        MessageTrait::template encode<cs_input>(requestInput),
        createResponseMsgHandlerCallback(std::move(callback)),
        callStatus
        );
}

template<class MessageTrait>
template <class OperationOrOutput,
         std::enable_if_t<
             std::is_base_of_v<cs_output, OperationOrOutput> ||
                 std::is_base_of_v<cs_operation, OperationOrOutput>,
             bool> >
RegID QueueingServiceProxy<MessageTrait>::sendRequestAsync(
    ResponseProcessingCallback<OperationOrOutput> callback,
    ActionCallStatus* callStatus
    )
{
    return _requester->sendRequestAsync(
        MessageTrait::template getOperationID<OperationOrOutput>(),
        {},
        createResponseMsgHandlerCallback(std::move(callback)),
        callStatus
        );
}

template<class MessageTrait>
template <class OperationOrOutput,
         std::enable_if_t<
             std::is_base_of_v<cs_output, OperationOrOutput> ||
                 std::is_base_of_v<cs_operation, OperationOrOutput>,
             bool> >
typename QueueingServiceProxy<MessageTrait>::template
    ResponsePtrType<OperationOrOutput>
    QueueingServiceProxy<MessageTrait>::sendRequest(
        const std::shared_ptr<cs_input> &requestInput,
        RequestTimeoutMs timeout,
        ActionCallStatus* callStatus
        )
{
    return sendRequest<OperationOrOutput>(
        MessageTrait::template getOperationID<OperationOrOutput>(),
        MessageTrait::template encode<cs_input>(requestInput),
        timeout,
        callStatus
        );
}

template<class MessageTrait>
template <class OperationOrOutput,
         std::enable_if_t<
             std::is_base_of_v<cs_output, OperationOrOutput> ||
                 std::is_base_of_v<cs_operation, OperationOrOutput>,
             bool> >
typename QueueingServiceProxy<MessageTrait>::template
    ResponsePtrType<OperationOrOutput>
QueueingServiceProxy<MessageTrait>::sendRequest(
        RequestTimeoutMs timeout,
        ActionCallStatus* callStatus
        )
{
    return sendRequest<OperationOrOutput>(
               MessageTrait::template getOperationID<OperationOrOutput>(),
               {},
               timeout,
               callStatus
        );
}

template<class MessageTrait>
template<class OperationOrOutput>
typename QueueingServiceProxy<MessageTrait>::template
    ResponsePtrType<OperationOrOutput>
    QueueingServiceProxy<MessageTrait>::sendRequest(
    OpID actionID,
    const CSMsgContentBasePtr& requestInput,
    RequestTimeoutMs timeout,
    ActionCallStatus* callStatus
    )
{
    auto callStatus_ = ActionCallStatus::FailedUnknown;
    auto rawResponse = _requester->sendRequest(
        actionID,
        requestInput,
        timeout,
        &callStatus_
        );

    if(callStatus) { *callStatus = callStatus_; }

    if(callStatus_ == ActionCallStatus::Success)
    {
        return getResposne<OperationOrOutput>(rawResponse);
    }
    else
    {
        return {};
    }
}


}
}

#endif
