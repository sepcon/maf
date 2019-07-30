#pragma once

#include "interfaces/CSDefines.h"
#include "ServiceProxyBase.h"
#include "thaf/messaging/Component.h"
#include "thaf/messaging/BasicMessages.h"
#include "thaf/utils/debugging/Debug.h"

namespace thaf {
namespace messaging {

template<class MessageTrait>
class QueueingServiceProxy : public ServiceProxyBase
{
    friend class ClientBase;
    using ListOfInterestedComponents = nstl::SyncObject<std::set<ComponentRef, std::less<ComponentRef>>>;
public:
    template <class SpecificMsgContent>
    using PayloadProcessCallback = std::function<void(const std::shared_ptr<SpecificMsgContent>&)>;

    template<class IncomingMsgContent>
    RegID sendStatusChangeRegister(OpID propertyID, PayloadProcessCallback<IncomingMsgContent> callback);

    template<class IncomingMsgContent>
    RegID sendActionRequest
    (
            const CSMsgContentPtr& outgoingData,
            PayloadProcessCallback<IncomingMsgContent> callback = nullptr
            );

    template<class IncomingMsgContent>
    std::shared_ptr<IncomingMsgContent> sendActionRequestSync
    (
            const CSMsgContentPtr& outgoingData,
            unsigned long maxWaitTimeMs = THAF_MAX_OPERATION_WAIT_MS
            );

    template<class IncomingMsgContent>
    bool sendActionRequestSync
    (
            const CSMsgContentPtr& outgoingData,
            PayloadProcessCallback<IncomingMsgContent> callback,
            unsigned long maxWaitTimeMs = THAF_MAX_OPERATION_WAIT_MS
            );

    ~QueueingServiceProxy(){}
    QueueingServiceProxy(ServiceID sid, ClientInterface *client);
protected:
    bool updateServiceStatusToComponent(ComponentRef compref, Availability oldStatus, Availability newStatus);
    void addInterestedComponent(ComponentRef compref);
    void onServiceStatusChanged(ServiceID sid, Availability oldStatus, Availability newStatus) override;
    template<class IncomingMsgContent>
    CSMessageHandlerCallback createMsgHandlerCallback(PayloadProcessCallback<IncomingMsgContent> callback, bool sync = false);

    ListOfInterestedComponents _listComponents;
    Availability _serviceStatus = Availability::Unavailable ;
};

template<class MessageTrait>
QueueingServiceProxy<MessageTrait>::QueueingServiceProxy(ServiceID sid, ClientInterface* client) :
    ServiceProxyBase (sid, client)
{
    auto compref = Component::getComponentRef();
    assert(compref && "Plase call createProxy/createStub on thread of a messaging::Component");
    addInterestedComponent(compref);
}

template<class MessageTrait>
void QueueingServiceProxy<MessageTrait>::addInterestedComponent(ComponentRef compref)
{
    auto lockListComps(_listComponents.pa_lock()); //Lock must be invoked here to protect both

    if(_serviceStatus == Availability::Available)
    {
        thafMsg("\n------------> Update Service status to component <<<<<<<< \n");
        updateServiceStatusToComponent(compref, Availability::Unavailable, Availability::Available);
    }
    auto comprefLock(compref->pa_lock());
    if(compref->get())
    {
        _listComponents->insert(compref);
        thafMsg("Component id [" << std::this_thread::get_id() << "] regsitered to listen to service status change");
    }
}

template<class MessageTrait>
bool QueueingServiceProxy<MessageTrait>::updateServiceStatusToComponent(ComponentRef compref, Availability oldStatus, Availability newStatus)
{
    auto comprefLock(compref->pa_lock());
    if(auto component = compref->get())
    {
        component->postMessage(messaging::createMessage<ServiceStatusMsg>(serviceID(), oldStatus, newStatus));
        return true;
    }
    return false;
}

template<class MessageTrait>
void QueueingServiceProxy<MessageTrait>::onServiceStatusChanged(ServiceID /*sid*/, Availability oldStatus, Availability newStatus)
{

    thafInfo("Service Status Changed: " << static_cast<int>(oldStatus) << " - " << static_cast<int>(newStatus));
    auto lock(_listComponents.pa_lock()); //this lock is responsible for both _listComponents protection and _serviceStatus
    if(_serviceStatus != newStatus)
    {
        _serviceStatus = newStatus;
        for(auto itCompref = _listComponents->begin(); itCompref != _listComponents->end(); )
        {
            if(updateServiceStatusToComponent(*itCompref, oldStatus, newStatus))
            {
                ++itCompref;
            }
            else
            {
                thafWarn("[[[[ - ]]]Component has no longer existed then has been removed![[[[ - ]]]\n");
                itCompref = _listComponents->erase(itCompref);
            }
        }
    }
}

template<class MessageTrait> template<class IncomingMsgContent>
CSMessageHandlerCallback QueueingServiceProxy<MessageTrait>::createMsgHandlerCallback(PayloadProcessCallback<IncomingMsgContent> callback, bool sync)
{
	auto compref = Component::getComponentRef();
    if(compref)
    {
        CSMessageHandlerCallback ipcMessageHandlerCB =
                [this, callback, sync, compref](const CSMessagePtr& msg){
            if(MessageTrait::template getOperationID<IncomingMsgContent>() == msg->operationID())
            {
                try
                {
                    auto dataCarrier = MessageTrait::template translate<IncomingMsgContent>(msg->content());
                    if(dataCarrier)
                    {
                        auto lock(compref->pa_lock());
                        if(auto component = compref->get())
                        {
                            if(sync)
                            {
                                callback(dataCarrier);
                            }
                            else
                            {
                                component->postMessage<CallbackExcMsg>( [dataCarrier, callback] {
                                    callback(dataCarrier);
                                });
                            }
                        }
                        else
                        {
                            thafWarn("The component that sending the request to [serviceID:" <<
                                     msg->serviceID() << "-operationID: " <<
                                     msg->operationID() << "] has no longer existed");
                        }
                    }
                    else
                    {
                        thafErr("Could not decode incomming message with id: " <<  msg->operationID());
                    }
                }
                catch(const std::exception& e)
                {
                    thafErr(e.what());
                }
            }
            else
            {
                thafErr("mismatched of OpID between client[" << MessageTrait::template getOperationID<IncomingMsgContent>() <<
                        "] and server[" << msg->operationID() << "]");
            }
        };
        return ipcMessageHandlerCB;
    }
    else
    {
        thafErr("Trying to create callback with no running component");
    }
    return nullptr;
}


template<class MessageTrait> template<class IncomingMsgContent>
RegID QueueingServiceProxy<MessageTrait>::sendActionRequest(const CSMsgContentPtr& outgoingData, PayloadProcessCallback<IncomingMsgContent> callback)
{
    assert((outgoingData->operationID() == MessageTrait::template getOperationID<IncomingMsgContent>())
           && "Please provide MessageContent that has same id with IncomingMessagecontent");
    outgoingData->makesureTransferable();
    return sendRequest(outgoingData, createMsgHandlerCallback(callback));
}

template<class MessageTrait> template<class IncomingMsgContent>
std::shared_ptr<IncomingMsgContent> QueueingServiceProxy<MessageTrait>::sendActionRequestSync(const CSMsgContentPtr &outgoingData, unsigned long maxWaitTimeMs)
{
    assert((outgoingData->operationID() == MessageTrait::template getOperationID<IncomingMsgContent>())
           && "Please provide MessageContent that has same id with IncomingMessagecontent");

    outgoingData->makesureTransferable();
    if(auto csMsg = sendRequestSync(outgoingData, maxWaitTimeMs))
    {
        return MessageTrait::template translate<IncomingMsgContent>(csMsg->content());
    }
    else
    {
        return {};
    }
}

template<class MessageTrait> template<class IncomingMsgContent>
bool QueueingServiceProxy<MessageTrait>::sendActionRequestSync(const CSMsgContentPtr &outgoingData,
        PayloadProcessCallback<IncomingMsgContent> callback,
        unsigned long maxWaitTimeMs )
{
    assert((outgoingData->operationID() == MessageTrait::template getOperationID<IncomingMsgContent>())
           && "Please provide MessageContent that has same id with IncomingMessagecontent");
    outgoingData->makesureTransferable();
    return sendRequestSync
            (
                outgoingData,
                createMsgHandlerCallback(callback, true),
                maxWaitTimeMs
                );
}

template<class MessageTrait> template<class IncomingMsgContent>
RegID QueueingServiceProxy<MessageTrait>::sendStatusChangeRegister(OpID propertyID, PayloadProcessCallback<IncomingMsgContent> callback)
{
    return ServiceProxyBase::sendStatusChangeRegister(propertyID, createMsgHandlerCallback(std::move(callback)));
}




}
}
