#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICEPROXY_IMPL_H
#define MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICEPROXY_IMPL_H

#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICEPROXY_H
#   include <maf/messaging/client-server/QueueingServiceProxy.h>
#endif

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

#ifndef mc_maf_assert_request_type
#define mc_maf_assert_request_type(RequestClass)        \
    static_assert (                                     \
        std::is_base_of_v<cs_request, RequestClass>,    \
        #RequestClass " must be a cs_request object"    \
        )
#endif

#ifndef mc_maf_assert_result_type
#define mc_maf_assert_result_type(ResultClass)          \
    static_assert (                                     \
        std::is_base_of_v<cs_result, ResultClass>,      \
        #ResultClass " must be a cs_result object"      \
        )
#endif


namespace maf { using logging::Logger;
namespace messaging {

template<class MessageTrait>
std::shared_ptr<QueueingServiceProxy<MessageTrait>>
QueueingServiceProxy<MessageTrait>::createProxy(
    const ConnectionType &contype,
    const Address &addr,
    ServiceID sid
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
        Logger::fatal("Failed to get Client with connection type ", contype, " and address ", addr.dump(-1));
    }
    return {};
}

template<class MessageTrait>
QueueingServiceProxy<MessageTrait>::QueueingServiceProxy(
        std::shared_ptr<ServiceRequesterInterface> requester
        ) : _requester{std::move(requester)} {}

template<class MessageTrait>
ServiceID QueueingServiceProxy<MessageTrait>::serviceID() const
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
QueueingServiceProxy<MessageTrait>::onServerStatusChanged(Availability oldStatus, Availability newStatus)
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
    ServiceID /*sid*/,
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
                auto operationID = MessageTrait::template getOperationID<IncomingMsgContent>();
                auto consumable = MessageTrait::template decode<IncomingMsgContent>(msgContent);
                if(consumable)
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
                        Logger::warn(
                                    "The component that sending the request of operationID: [",
                                    operationID,
                                    "] has no longer existed"
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
    else
    {
        if(!Component::getActiveWeakPtr().lock())
        {
            Logger::error("Trying to create callback with no running component");
        }
    }
    return nullptr;
}

template<class MessageTrait> template<class Status>
RegID QueueingServiceProxy<MessageTrait>::registerStatus(
        PayloadProcessCallback<Status> callback
        )
{
    mc_maf_assert_status_type(Status);

    return _requester->registerStatus(
                MessageTrait::template getOperationID<Status>(),
                createMsgHandlerAsyncCallback(std::move(callback))
                );
}

template<class MessageTrait>
void QueueingServiceProxy<MessageTrait>::unregisterStatus(const RegID& regID)
{
    _requester->unregisterStatus(regID);
}

template<class MessageTrait>
void QueueingServiceProxy<MessageTrait>::unregisterStatusAll(OpID propertyID)
{
    _requester->unregisterStatusAll(propertyID);
}

template<class MessageTrait> template<class Status>
RegID QueueingServiceProxy<MessageTrait>::getStatusAsync(
        CSMessageContentHandlerCallback callback
        )
{
    mc_maf_assert_status_type(Status);

    return _requester->getStatusAsync(
                MessageTrait::template getOperationID<Status>(),
                createMsgHandlerAsyncCallback(std::move(callback))
                );
}

template<class MessageTrait> template<class Status>
std::shared_ptr<Status> QueueingServiceProxy<MessageTrait>::getStatus(
        unsigned long maxWaitTimeMs
        )
{
    mc_maf_assert_status_type(Status);

    auto propertyID = MessageTrait::template getOperationID<Status>();
    if(auto msgContent = _requester->getStatus(
                propertyID,
                maxWaitTimeMs
                )
            )
    {
        return MessageTrait::template decode<Status>(msgContent);
    }
    else
    {
        return {};
    }
}

template<class MessageTrait>
template<class action_result,
         std::enable_if_t<std::is_base_of_v<cs_result, action_result>, bool>
         >
RegID QueueingServiceProxy<MessageTrait>::requestActionAsync(
    const std::shared_ptr<cs_request> &requestInput,
    PayloadProcessCallback<action_result> callback
    )
{
    mc_maf_assert_result_type(action_result);

    return _requester->requestActionAsync(
        MessageTrait::template getOperationID<action_result>(),
        MessageTrait::template encode<cs_request>(requestInput),
        createMsgHandlerAsyncCallback(callback)
        );
}

template<class MessageTrait>
template<class action_result,
         std::enable_if_t<std::is_base_of_v<cs_result, action_result>, bool>
         >
RegID QueueingServiceProxy<MessageTrait>::requestActionAsync(
        PayloadProcessCallback<action_result> callback
        )
{
    mc_maf_assert_result_type(action_result);
    return _requester->requestActionAsync(
        MessageTrait::template getOperationID<action_result>(),
        {},
        createMsgHandlerAsyncCallback(std::move(callback))
        );
}

template<class MessageTrait>
mc_maf_tpl_enable_if_is_base_of(cs_request, request_input)
RegID QueueingServiceProxy<MessageTrait>::requestActionAsync(
        const std::shared_ptr<request_input> &requestInput
        )
{
    return _requester->requestActionAsync(
        MessageTrait::template getOperationID<request_input>(),
        MessageTrait::template encode(requestInput),
        {});
}

template<class MessageTrait>
mc_maf_tpl_enable_if_is_base_of(cs_result, action_result)
std::shared_ptr<action_result> QueueingServiceProxy<MessageTrait>::requestAction(
    const std::shared_ptr<cs_request> &requestInput,
    unsigned long maxWaitTimeMs
    )
{
    mc_maf_assert_result_type(action_result);
    if(auto msgContent = _requester->requestAction(
            MessageTrait::template getOperationID<action_result>(),
            MessageTrait::template encode<cs_request>(requestInput),
            maxWaitTimeMs
            )
        )
    {
        return MessageTrait::template decode<action_result>(msgContent);
    }
    else
    {
        return {};
    }
}

template<class MessageTrait>
mc_maf_tpl_enable_if_is_base_of(cs_result, action_result)
std::shared_ptr<action_result>
QueueingServiceProxy<MessageTrait>::requestAction(unsigned long maxWaitTimeMs)
{
    mc_maf_assert_result_type(action_result);

    auto actionID = MessageTrait::template getOperationID<action_result>();
    if(auto msgContent = _requester->requestAction(
                actionID,
                {},
                maxWaitTimeMs
                )
            )
    {
        return MessageTrait::template decode<action_result>( msgContent );
    }
    else
    {
        return {};
    }
}

template<class MessageTrait>
mc_maf_tpl_enable_if_is_base_of(cs_request, request_input)
void QueueingServiceProxy<MessageTrait>::requestAction(
        const std::shared_ptr<request_input>& input,
        unsigned long maxWaitTimeMs)
{

    auto actionID = MessageTrait::template getOperationID<request_input>();
    _requester->requestAction(
                    actionID,
                    input,
                    maxWaitTimeMs
                    );
}


}
}

#endif
