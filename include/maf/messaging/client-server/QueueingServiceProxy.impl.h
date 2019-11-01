#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICEPROXY_IMPL_H
#define MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICEPROXY_IMPL_H

#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICEPROXY_H
#   include "QueueingServiceProxy.h"
#endif
#include <maf/logging/Logger.h>

namespace maf { using logging::Logger;
namespace messaging {

template<class MessageTrait>
void
QueueingServiceProxy<MessageTrait>::addInterestedComponent(ComponentRef compref)
{
    if(auto comp = compref.lock())
    {
        std::lock_guard lock(_components); //Lock must be invoked here to protect both
        auto insertResult = _components->insert(compref);
        if(insertResult.second) // means that insertion took place
        {
            if(getClient()->getServiceStatus(serviceID()) == Availability::Available)
            {
                updateServiceStatusToComponent(compref, Availability::Unavailable, Availability::Available);
            }
        }
        else // Component had already been registered to Proxy
        {
            Logger::warn("The component: " ,  comp->name() ,  " had already got one instance of proxy for service id [" ,  serviceID() , "]");
        }
    }
    else
    {
        Logger::error("Trying to get reference to service proxy from non-component thread: ServiceID: " ,  serviceID());
    }
}

template<class MessageTrait>
void
QueueingServiceProxy<MessageTrait>::onServerStatusChanged(Availability oldStatus, Availability newStatus)
{
    ServiceProxyBase::onServerStatusChanged(oldStatus, newStatus);
    if(newStatus != Availability::Available)
    { // Notify observers that service is not available as well
        onServiceStatusChanged(serviceID(), oldStatus, newStatus);
    }
}

template<class MessageTrait>
bool
QueueingServiceProxy<MessageTrait>::updateServiceStatusToComponent
    (
        ComponentRef compref,
        Availability oldStatus,
        Availability newStatus
        )
{
    if(auto component = compref.lock())
    {
        component->postMessage(messaging::makeMessage<ServiceStatusMsg>(serviceID(), oldStatus, newStatus));
        return true;
    }
    return false;
}

template<class MessageTrait>
void
QueueingServiceProxy<MessageTrait>::onServiceStatusChanged(ServiceID sid, Availability oldStatus, Availability newStatus)
{
    assert(sid == serviceID());
    Logger::info("Service id " ,  serviceID() ,  " has changed Status: " ,  static_cast<int>(oldStatus) ,  " - " ,  static_cast<int>(newStatus));
    std::lock_guard lock(_components);
    for(auto itCompref = _components->begin(); itCompref != _components->end(); )
    {
        if(updateServiceStatusToComponent(*itCompref, oldStatus, newStatus))
        {
            ++itCompref;
        }
        else
        {
            Logger::warn("[[[[ - ]]]Component has no longer existed then has been removed![[[[ - ]]]\n");
            itCompref = _components->erase(itCompref);
        }
    }
}

template<class MessageTrait> template<class IncomingMsgContent>
CSMessageHandlerCallback
QueueingServiceProxy<MessageTrait>::createMsgHandlerAsyncCallback(PayloadProcessCallback<IncomingMsgContent> callback)
{
    if(callback)
    {
        auto compref = Component::getActiveWeakPtr(); //compref must be copied into lambda

        CSMessageHandlerCallback ipcMessageHandlerCB =
            [this, callback, compref](const CSMessagePtr& msg){
                if(MessageTrait::template getOperationID<IncomingMsgContent>() == msg->operationID())
                {
                    try
                    {
                        auto dataCarrier = MessageTrait::template translate<IncomingMsgContent>(msg->content());
                        if(dataCarrier)
                        {
                            if(auto component = compref.lock())
                            {
                                // Request the requesting Component to execute the callback but
                                component->postMessage<CallbackExcMsg>( [dataCarrier, callback] {
                                    callback(dataCarrier);
                                });
                            }
                            else
                            {
                                Logger::warn("The component that sending the request to [serviceID:" ,
                                        msg->serviceID() ,  "-operationID: " ,
                                        msg->operationID() ,  "] has no longer existed");
                            }
                        }
                        else
                        {
                            Logger::error("Could not decode incomming message with id: " ,   msg->operationID());
                        }
                    }
                    catch(const std::exception& e)
                    {
                        Logger::error(e.what());
                    }
                }
                else
                {
                    Logger::error("mismatched of OpID between client[" ,  MessageTrait::template getOperationID<IncomingMsgContent>() ,
                           "] and server[" ,  msg->operationID() ,  "]");
                }
            };
        return ipcMessageHandlerCB;
    }
    else
    {
        if(!Component::getActiveWeakPtr().lock()) { Logger::error("Trying to create callback with no running component"); }
    }
    return nullptr;
}


template<class MessageTrait> template<class IncomingMsgContent>
RegID
QueueingServiceProxy<MessageTrait>::sendRequest(const CSMsgContentPtr& outgoingData, PayloadProcessCallback<IncomingMsgContent> callback)
{
    assert(
        outgoingData->operationID() == MessageTrait::template getOperationID<IncomingMsgContent>()
        && "Please provide MessageContent that has same id with IncomingMessagecontent"
        );

    outgoingData->makesureTransferable();
    return _MyBase::sendRequest(outgoingData, createMsgHandlerAsyncCallback(callback));
}

template<class MessageTrait> template<class IncomingMsgContent>
std::shared_ptr<IncomingMsgContent>
QueueingServiceProxy<MessageTrait>::sendRequestSync(const CSMsgContentPtr &outgoingData, unsigned long maxWaitTimeMs)
{
    assert((outgoingData->operationID() == MessageTrait::template getOperationID<IncomingMsgContent>())
           && "Please provide MessageContent that has same id with IncomingMessagecontent");

    outgoingData->makesureTransferable();
    if(auto csMsg = _MyBase::sendRequestSync(outgoingData, maxWaitTimeMs))
    {
        return MessageTrait::template translate<IncomingMsgContent>(csMsg->content());
    }
    else
    {
        return {};
    }
}

template<class MessageTrait> template<class IncomingMsgContent>
bool
QueueingServiceProxy<MessageTrait>::sendRequestSync(const CSMsgContentPtr &outgoingData,
                                                    PayloadProcessCallback<IncomingMsgContent> callback,
                                                    unsigned long maxWaitTimeMs )
{
    assert((outgoingData->operationID() == MessageTrait::template getOperationID<IncomingMsgContent>())
           && "Please provide MessageContent that has same id with IncomingMessagecontent");
    outgoingData->makesureTransferable();

    auto response = sendRequestSync<IncomingMsgContent>(outgoingData, maxWaitTimeMs);
    if(response)
    {
        callback(response);
        return true;
    }
    else
    {
        return false;
    }
}


template<class MessageTrait> template<class IncomingMsgContent>
RegID QueueingServiceProxy<MessageTrait>::sendStatusChangeRegister(PayloadProcessCallback<IncomingMsgContent> callback)
{
    return ServiceProxyBase::sendStatusChangeRegister(MessageTrait::template getOperationID<IncomingMsgContent>(), createMsgHandlerAsyncCallback(std::move(callback)));
}

template<class MessageTrait> template<class IncomingMsgContent>
std::shared_ptr<IncomingMsgContent>
QueueingServiceProxy<MessageTrait>::sendRequestSync(unsigned long maxWaitTimeMs)
{
    auto csMsgResponse = _MyBase::sendRequestSync(MessageTrait::template getOperationID<IncomingMsgContent>(), {}, maxWaitTimeMs);
    if(csMsgResponse && csMsgResponse->content())
    {
        return MessageTrait::template translate<IncomingMsgContent>(csMsgResponse->content());
    }
    else
    {
        return {};
    }
}

template<class MessageTrait> template<class IncomingMsgContent>
RegID
QueueingServiceProxy<MessageTrait>::sendRequest(PayloadProcessCallback<IncomingMsgContent> callback)
{
    return _MyBase::sendRequest(MessageTrait::template getOperationID<IncomingMsgContent>(), {}, createMsgHandlerAsyncCallback(callback));
}


}
}

#endif
