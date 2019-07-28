#pragma once

#include "ServiceProxyBase.h"
#include "thaf/messaging/Component.h"
#include "thaf/messaging/BasicMessages.h"
#include "thaf/utils/debugging/Debug.h"

namespace thaf {
namespace messaging {

template<class MessageTrait, class ManagerClient>
class QueueingServiceProxy : public ServiceProxyBase
{
    using MyType = QueueingServiceProxy<MessageTrait, ManagerClient>;
    using MyPtr = std::shared_ptr<MyType>;
    using ListOfInterestedComponents = stl::SyncObject<std::set<ComponentRef, std::less<ComponentRef>>>;
public:
    template <class SpecificMsgContent>
    using PayloadProcessCallback = std::function<void(const std::shared_ptr<SpecificMsgContent>&)>;

    static MyPtr createProxy(ServiceID sid);

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
            unsigned long maxWaitTimeMs = static_cast<unsigned long>(-1)
            );

    template<class IncomingMsgContent>
    bool sendActionRequestSync
    (
            const CSMsgContentPtr& outgoingData,
            PayloadProcessCallback<IncomingMsgContent> callback,
            unsigned long maxWaitTimeMs = static_cast<unsigned long>(-1)
            );

    ~QueueingServiceProxy(){}
protected:
    QueueingServiceProxy(ServiceID sid);
    bool updateServiceStatusToComponent(ComponentRef compref, Availability oldStatus, Availability newStatus);
    void addInterestedComponent(ComponentRef compref);
    void onServiceStatusChanged(ServiceID sid, Availability oldStatus, Availability newStatus) override;
    template<class IncomingMsgContent>
    CSMessageHandlerCallback createMsgHandlerCallback(PayloadProcessCallback<IncomingMsgContent> callback, bool sync = false);

    ListOfInterestedComponents _listComponents;
    Availability _serviceStatus = Availability::Unavailable ;
};

template<class MessageTrait, class ManagerClient>
typename QueueingServiceProxy<MessageTrait, ManagerClient>::MyPtr QueueingServiceProxy<MessageTrait, ManagerClient>::createProxy(ServiceID sid)
{
    static std::mutex creatingMutex;
    std::lock_guard lock(creatingMutex);
    auto serviceRequester = ManagerClient::instance().getServiceRequester(sid);
    if(!serviceRequester)
    {
        serviceRequester.reset(new MyType(sid));
        ManagerClient::instance().registerServiceRequester(serviceRequester);
    }

    auto proxy = std::static_pointer_cast<MyType>(serviceRequester);
    auto compref = Component::getComponentRef();
    proxy->addInterestedComponent(compref);

    return proxy;
}

template<class MessageTrait, class ManagerClient>
QueueingServiceProxy<MessageTrait, ManagerClient>::QueueingServiceProxy(ServiceID sid) :
    ServiceProxyBase (sid, &ManagerClient::instance())
{
}

template<class MessageTrait, class ManagerClient>
void QueueingServiceProxy<MessageTrait, ManagerClient>::addInterestedComponent(ComponentRef compref)
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

template<class MessageTrait, class ManagerClient>
bool QueueingServiceProxy<MessageTrait, ManagerClient>::updateServiceStatusToComponent(ComponentRef compref, Availability oldStatus, Availability newStatus)
{
    auto comprefLock(compref->pa_lock());
    if(auto component = compref->get())
    {
        component->postMessage(messaging::createMessage<ServiceStatusMsg>(serviceID(), oldStatus, newStatus));
        return true;
    }
    return false;
}

template<class MessageTrait, class ManagerClient>
void QueueingServiceProxy<MessageTrait, ManagerClient>::onServiceStatusChanged(ServiceID /*sid*/, Availability oldStatus, Availability newStatus)
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

template<class MessageTrait, class ManagerClient> template<class IncomingMsgContent>
CSMessageHandlerCallback QueueingServiceProxy<MessageTrait, ManagerClient>::createMsgHandlerCallback(PayloadProcessCallback<IncomingMsgContent> callback, bool sync)
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


template<class MessageTrait, class ManagerClient> template<class IncomingMsgContent>
RegID QueueingServiceProxy<MessageTrait, ManagerClient>::sendActionRequest(const CSMsgContentPtr& outgoingData, PayloadProcessCallback<IncomingMsgContent> callback)
{
    assert((outgoingData->operationID() == MessageTrait::template getOperationID<IncomingMsgContent>())
           && "Please provide MessageContent that has same id with IncomingMessagecontent");
    outgoingData->makesureTransferable();
    return sendRequest(outgoingData, createMsgHandlerCallback(callback));
}

template<class MessageTrait, class ManagerClient> template<class IncomingMsgContent>
std::shared_ptr<IncomingMsgContent> QueueingServiceProxy<MessageTrait, ManagerClient>::sendActionRequestSync(const CSMsgContentPtr &outgoingData, unsigned long maxWaitTimeMs)
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

template<class MessageTrait, class ManagerClient> template<class IncomingMsgContent>
bool QueueingServiceProxy<MessageTrait, ManagerClient>::sendActionRequestSync(const CSMsgContentPtr &outgoingData,
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

template<class MessageTrait, class ManagerClient> template<class IncomingMsgContent>
RegID QueueingServiceProxy<MessageTrait, ManagerClient>::sendStatusChangeRegister(OpID propertyID, PayloadProcessCallback<IncomingMsgContent> callback)
{
    return ServiceProxyBase::sendStatusChangeRegister(propertyID, createMsgHandlerCallback(std::move(callback)));
}




}
}
