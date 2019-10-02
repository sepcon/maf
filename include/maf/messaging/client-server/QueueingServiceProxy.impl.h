#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICEPROXY_IMPL_H
#define MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICEPROXY_IMPL_H

#ifndef MAF_MESSAGING_CLIENT_SERVER_QUEUEINGSERVICEPROXY_H
#   include "QueueingServiceProxy.h"
#endif

namespace maf {
namespace messaging {

template<class MessageTrait>
void
QueueingServiceProxy<MessageTrait>::addInterestedComponent(ComponentRef compref)
{
    if(auto comp = compref.lock())
    {
        auto lockListComps = _listComponents.a_lock(); //Lock must be invoked here to protect both
        auto insertResult = _listComponents->insert(compref);
        if(insertResult.second) // means that insertion took place
        {
            if(getClient()->getServiceStatus(serviceID()) == Availability::Available)
            {
                updateServiceStatusToComponent(compref, Availability::Unavailable, Availability::Available);
            }
        }
        else // Component had already been registered to Proxy
        {
            mafWarn("The component: " << comp->name() << " had already got one instance of proxy for service id [" << serviceID() <<"]");
        }
    }
    else
    {
        mafErr("Trying to get reference to service proxy from non-component thread: ServiceID: " << serviceID());
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
        component->postMessage(messaging::createMessage<ServiceStatusMsg>(serviceID(), oldStatus, newStatus));
        return true;
    }
    return false;
}

template<class MessageTrait>
void
QueueingServiceProxy<MessageTrait>::onServiceStatusChanged(ServiceID sid, Availability oldStatus, Availability newStatus)
{
    assert(sid == serviceID());
    mafInfo("Service id " << serviceID() << " has changed Status: " << static_cast<int>(oldStatus) << " - " << static_cast<int>(newStatus));
    auto lock = _listComponents.a_lock();
    for(auto itCompref = _listComponents->begin(); itCompref != _listComponents->end(); )
    {
        if(updateServiceStatusToComponent(*itCompref, oldStatus, newStatus))
        {
            ++itCompref;
        }
        else
        {
            mafWarn("[[[[ - ]]]Component has no longer existed then has been removed![[[[ - ]]]\n");
            itCompref = _listComponents->erase(itCompref);
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
                                mafWarn("The component that sending the request to [serviceID:" <<
                                        msg->serviceID() << "-operationID: " <<
                                        msg->operationID() << "] has no longer existed");
                            }
                        }
                        else
                        {
                            mafErr("Could not decode incomming message with id: " <<  msg->operationID());
                        }
                    }
                    catch(const std::exception& e)
                    {
                        mafErr(e.what());
                    }
                }
                else
                {
                    mafErr("mismatched of OpID between client[" << MessageTrait::template getOperationID<IncomingMsgContent>() <<
                           "] and server[" << msg->operationID() << "]");
                }
            };
        return ipcMessageHandlerCB;
    }
    else
    {
        if(!Component::getActiveWeakPtr().lock()) { mafErr("Trying to create callback with no running component"); }
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
RegID
QueueingServiceProxy<MessageTrait>::sendStatusChangeRegister(OpID propertyID, PayloadProcessCallback<IncomingMsgContent> callback)
{
    return ServiceProxyBase::sendStatusChangeRegister(propertyID, createMsgHandlerAsyncCallback(std::move(callback)));
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
