#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICEPROXY_IMPL_H
#define MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICEPROXY_IMPL_H

#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICEPROXY_H
#   include <maf/messaging/client-server/QueueingServiceProxy.h>
#endif

#include <maf/messaging/client-server/MessageTraitBase.h>
#include <maf/messaging/client-server/ServiceStatusMsg.h>
#include <maf/messaging/client-server/ClientFactory.h>
#include <maf/messaging/BasicMessages.h>
#include <maf/logging/Logger.h>

#ifndef mc_maf_assert_status_type
#define mc_maf_assert_status_type(StatusClass)          \
    static_assert (                                     \
        std::is_base_of_v<cs_status, StatusClass>,      \
        "PropertyStatus must be a cs_status object"     \
        )
#endif

#ifndef mc_maf_assert_request_input_type
#define mc_maf_assert_request_input_type(RequestClass)        \
    static_assert (                                     \
        std::is_base_of_v<cs_input, RequestClass>,    \
        #RequestClass " must be a cs_input object"    \
        )
#endif

#ifndef mc_maf_assert_request_output_type
#define mc_maf_assert_request_output_type(ResultClass)          \
    static_assert (                                     \
        std::is_base_of_v<cs_output, ResultClass>,      \
        #ResultClass " must be a cs_output object"      \
        )
#endif


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
    if(auto client = ClientFactory::instance().getClient(contype, addr))
    {
        if(auto requester = client->getServiceRequester(sid))
        {
            auto proxy = std::shared_ptr<QueueingServiceProxy<MessageTrait>>
                {
                    new QueueingServiceProxy<MessageTrait>(requester)
                };
            requester->addServiceStatusObserver(proxy);
            return proxy;
        }
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
        component->postMessage(
            messaging::makeCompMessage<ServiceStatusMsg>(
                _requester->serviceID(),
                oldStatus,
                newStatus
                )
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

template<class MessageTrait> template<class IncomingMsgContent>
CSMessageContentHandlerCallback
QueueingServiceProxy<MessageTrait>::createMsgHandlerAsyncCallback(
        PayloadProcessCallback<IncomingMsgContent> callback
        )
{
    if(callback)
    {
        // NOTE: this is not main component
        // Every component that emits the request should handle the request
        // itsef, then here we try to use Component::getActiveWeakPtr() instead
        // of _compref
        // compref must be captured by value instead of reference
        CSMessageContentHandlerCallback ipcMessageHandlerCB =
                [
                callback = std::move(callback),
                compref = Component::getActiveWeakPtr()
                ] (const CSMsgContentBasePtr& msgContent) {
            try
            {
                MessageTraitBase::EncodeDecodeStatus decodeStatus;
                auto operationID = MessageTrait::template
                    getOperationID<IncomingMsgContent>();
                auto consumable = MessageTrait::template
                    decode<IncomingMsgContent>(msgContent, &decodeStatus);
                if(decodeStatus != MessageTraitBase::MalformInput)
                {
                    if(auto component = compref.lock())
                    {
                        // Request the requesting Component to execute the callback but
                        component->postMessage<CallbackExcMsg>(
                                    [
                                    consumable,
                                    callback = std::move(callback)
                                    ]
                        {
                            callback(consumable);
                        });
                    }
                    else
                    {
                        Logger::warn (
                            "The component that sending the request of "
                            "operationID: [", operationID, "] has no longer"
                            " existed"
                            );
                    }
                }
                else
                {
                    Logger::error(
                                "Could not decode incomming message with id: ",
                                operationID
                                );
                }
            }
            catch(const std::exception& e)
            {
                Logger::error(e.what());
            }
        };
        return ipcMessageHandlerCB;
    }

    return {}; //don't care about result
}

template<class MessageTrait> template<class property_status>
RegID QueueingServiceProxy<MessageTrait>::registerStatus(
    PayloadProcessCallback<property_status> callback
    )
{
    mc_maf_assert_status_type(property_status);
    auto propertyID = MessageTrait::template getOperationID<property_status>();
    if(callback)
    {
        return _requester->registerStatus(
            propertyID,
            createMsgHandlerAsyncCallback(std::move(callback))
            );
    }
    else
    {
        Logger::error("Registering status id[, ",
                      propertyID,
                      "] failed, Please provide non-empty callback");
    }
    return {};
}

template<class MessageTrait> template<class signal_attributes>
RegID QueueingServiceProxy<MessageTrait>::registerSignal(
    PayloadProcessCallback<signal_attributes> callback
    )
{
    static_assert (
        std::is_base_of_v<cs_attributes, signal_attributes>,
        "signal_attributes must be a cs_attributes "
        );
    auto signalID = MessageTrait::template getOperationID<signal_attributes>();
    if(callback)
    {
        return _requester->registerSignal(
            signalID,
            createMsgHandlerAsyncCallback(std::move(callback))
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

template<class MessageTrait> template<class signal_class>
RegID QueueingServiceProxy<MessageTrait>::registerSignal(
    std::function<void(void)> callback
    )
{
    static_assert (
        std::is_base_of_v<cs_signal, signal_class>,
        "signal_class must be a cs_signal "
        );

    auto signalID = MessageTrait::template getOperationID<signal_class>();
    if(callback)
    {
        return _requester->registerSignal(
            signalID,
            [
                signalID,
                callback = std::move(callback),
                compref = Component::getActiveWeakPtr()
        ] (const auto&) mutable {
                if(auto component = compref.lock())
                {
                    // Request the requesting Component to execute the callback but
                    component->postMessage<CallbackExcMsg>(std::move(callback));
                }
                else
                {
                    Logger::warn(
                        "The component that sending the request of operationID: "
                        "[", signalID, "] has no longer existed"
                        );
                }
            }
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
void QueueingServiceProxy<MessageTrait>::unregisterStatus(const RegID& regID)
{
    _requester->unregisterStatus(regID);
}

template<class MessageTrait>
void QueueingServiceProxy<MessageTrait>::unregisterStatusAll(const OpID& propertyID)
{
    _requester->unregisterStatusAll(propertyID);
}

template<class MessageTrait> template<class property_status>
RegID QueueingServiceProxy<MessageTrait>::getStatusAsync(
        CSMessageContentHandlerCallback callback
        )
{
    mc_maf_assert_status_type(property_status);

    return _requester->getStatusAsync(
                MessageTrait::template getOperationID<property_status>(),
                createMsgHandlerAsyncCallback(std::move(callback))
                );
}

template<class MessageTrait> template<class property_status>
std::shared_ptr<property_status> QueueingServiceProxy<MessageTrait>::getStatus(
        unsigned long maxWaitTimeMs
        )
{
    mc_maf_assert_status_type(property_status);

    auto propertyID = MessageTrait::template getOperationID<property_status>();
    if(auto msgContent = _requester->getStatus(
                propertyID,
                maxWaitTimeMs
                )
            )
    {
        return MessageTrait::template decode<property_status>(msgContent);
    }
    else
    {
        return {};
    }
}

template<class MessageTrait>
template<class request_output,
         std::enable_if_t<std::is_base_of_v<cs_output, request_output>, bool>
         >
RegID QueueingServiceProxy<MessageTrait>::sendRequestAsync(
    const std::shared_ptr<cs_input> &requestInput,
    PayloadProcessCallback<request_output> callback
    )
{
    mc_maf_assert_request_output_type(request_output);

    return _requester->sendRequestAsync(
        MessageTrait::template getOperationID<request_output>(),
        MessageTrait::template encode<cs_input>(requestInput),
        createMsgHandlerAsyncCallback(callback)
        );
}

template<class MessageTrait>
template<class request_output,
         std::enable_if_t<std::is_base_of_v<cs_output, request_output>, bool>
         >
RegID QueueingServiceProxy<MessageTrait>::sendRequestAsync(
        PayloadProcessCallback<request_output> callback
        )
{
    mc_maf_assert_request_output_type(request_output);
    return _requester->sendRequestAsync(
        MessageTrait::template getOperationID<request_output>(),
        {},
        createMsgHandlerAsyncCallback(std::move(callback))
        );
}

template<class MessageTrait>
mc_maf_tpl_enable_if_is_base_of(cs_input, request_input)
RegID QueueingServiceProxy<MessageTrait>::sendRequestAsync(
        const std::shared_ptr<request_input> &requestInput
        )
{
    return _requester->sendRequestAsync(
        MessageTrait::template getOperationID<request_input>(),
        MessageTrait::template encode(requestInput),
        {});
}

template<class MessageTrait>
mc_maf_tpl_enable_if_is_base_of(cs_request, request_class)
RegID QueueingServiceProxy<MessageTrait>::sendRequestAsync()
{
    return _requester->sendRequestAsync(
        MessageTrait::template getOperationID<request_class>(),
        {},
        {});
}

template<class MessageTrait>
mc_maf_tpl_enable_if_is_base_of(cs_output, request_output)
std::shared_ptr<request_output> QueueingServiceProxy<MessageTrait>::sendRequest(
    const std::shared_ptr<cs_input> &requestInput,
    unsigned long maxWaitTimeMs
    )
{
    mc_maf_assert_request_output_type(request_output);
    if(auto msgContent = _requester->sendRequest(
            MessageTrait::template getOperationID<request_output>(),
            MessageTrait::template encode<cs_input>(requestInput),
            maxWaitTimeMs
            )
        )
    {
        return MessageTrait::template decode<request_output>(msgContent);
    }
    else
    {
        return {};
    }
}

template<class MessageTrait>
mc_maf_tpl_enable_if_is_base_of(cs_output, request_output)
std::shared_ptr<request_output>
QueueingServiceProxy<MessageTrait>::sendRequest(unsigned long maxWaitTimeMs)
{
    mc_maf_assert_request_output_type(request_output);

    auto actionID = MessageTrait::template getOperationID<request_output>();
    if(auto msgContent = _requester->sendRequest(
                actionID,
                {},
                maxWaitTimeMs
                )
            )
    {
        return MessageTrait::template decode<request_output>( msgContent );
    }
    else
    {
        return {};
    }
}

template<class MessageTrait>
mc_maf_tpl_enable_if_is_base_of(cs_input, request_input)
void QueueingServiceProxy<MessageTrait>::sendRequest(
        const std::shared_ptr<request_input>& input,
        unsigned long maxWaitTimeMs)
{
    mc_maf_assert_request_input_type(request_input);

    auto actionID = MessageTrait::template getOperationID<request_input>();
    _requester->sendRequest(
                    actionID,
                    input,
                    maxWaitTimeMs
                    );
}

template<class MessageTrait>
mc_maf_tpl_enable_if_is_base_of(cs_request, request_class)
    void QueueingServiceProxy<MessageTrait>::sendRequest(
        unsigned long maxWaitTimeMs
        )
{

    auto actionID = MessageTrait::template getOperationID<request_class>();
    _requester->sendRequest(
        actionID,
        {},
        maxWaitTimeMs
        );
}

}
}

#endif
