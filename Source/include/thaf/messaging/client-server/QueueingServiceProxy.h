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
    using MyRef = std::shared_ptr<MyType>;
    using ListOfInterestedComponents = stl::SyncObject<std::set<ComponentRef>>;
public:
    template <class SpecificMsgContent>
    using PayloadProcessCallback = std::function<void(const std::shared_ptr<SpecificMsgContent>&)>;

    static MyRef createProxy(ServiceID sid);

    template<class IncomingMsgContent>
    RegID sendStatusChangeRegister(OpID propertyID, PayloadProcessCallback<IncomingMsgContent> callback);

    template<class IncomingMsgContent>
    RegID sendActionRequest
    (
            const CSMsgContentPtr& outgoingData,
            PayloadProcessCallback<IncomingMsgContent> callback = nullptr
            );

    template<class IncomingMsgContent>
    bool sendActionRequestSync
    (
            const CSMsgContentPtr& outgoingData,
            PayloadProcessCallback<IncomingMsgContent> callback = nullptr,
            unsigned long maxWaitTimeMs = static_cast<unsigned long>(-1)
            );

    ~QueueingServiceProxy(){}
protected:
    QueueingServiceProxy(ServiceID sid);
    void addInterestedComponent(ComponentRef compref);
    void onServiceStatusChanged(ServiceID sid, Availability oldStatus, Availability newStatus) override;
    template<class IncomingMsgContent>
    CSMessageHandlerCallback createMsgHandlerCallback(PayloadProcessCallback<IncomingMsgContent> callback, bool sync = false);

    ListOfInterestedComponents _listComponents;
    std::atomic<Availability> _serviceStatus{Availability::Unavailable};
};

template<class MessageTrait, class ManagerClient>
typename QueueingServiceProxy<MessageTrait, ManagerClient>::MyRef QueueingServiceProxy<MessageTrait, ManagerClient>::createProxy(ServiceID sid)
{
    auto isNew = true;
    auto serviceRequester = ManagerClient::instance().getServiceRequeser(sid);
    if(!serviceRequester)
    {
        serviceRequester.reset(new MyType(sid));
        isNew = true;
    }

    auto proxy = std::static_pointer_cast<MyType>(serviceRequester);
    proxy->addInterestedComponent(Component::getComponentRef());

    //Registering service requester could notify back the change ServiceProvider status immediately
    //Then the call to registerServiceRequester must be invoked after addInterestedComponent
    if(isNew) ManagerClient::instance().registerServiceRequester(serviceRequester);

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
    auto comprefLock(compref->pa_lock());
    if(compref->get())
    {
        auto lock(_listComponents.pa_lock());
        _listComponents->insert(compref);
    }
}

template<class MessageTrait, class ManagerClient>
void QueueingServiceProxy<MessageTrait, ManagerClient>::onServiceStatusChanged(ServiceID /*sid*/, Availability oldStatus, Availability newStatus)
{

    thafInfo("Service Status Changed: " << static_cast<int>(oldStatus) << " - " << static_cast<int>(newStatus));
    if(_serviceStatus.load(std::memory_order_acquire) != newStatus)
    {
        _serviceStatus.store(newStatus, std::memory_order_release);
        auto lock(_listComponents.pa_lock());
        ComponentRef liveSavingRef;
        for(auto itCompref = _listComponents->begin(); itCompref != _listComponents->end(); )
        {
            auto comprefLock((*itCompref)->pa_lock());
            if(auto component = (*itCompref)->get())
            {
                component->postMessage(messaging::createMessage<ServiceStatusMsg>(oldStatus, newStatus));
                ++itCompref;
            }
            else
            {
                // Doing this to prevent the refcount to reach zero then the compptr object will be destroyed when it is
                // holding the mutex lock that will crash the program
                liveSavingRef = *itCompref;
                itCompref = _listComponents->erase(itCompref);
            }
        }
    }
}

template<class MessageTrait, class ManagerClient> template<class IncomingMsgContent>
CSMessageHandlerCallback QueueingServiceProxy<MessageTrait, ManagerClient>::createMsgHandlerCallback(PayloadProcessCallback<IncomingMsgContent> callback, bool sync)
{
    if(auto compref = Component::getComponentRef())
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
    assert((outgoingData->operationID() == MessageTrait::template getOperationID<IncomingMsgContent>()) && "Please provide MessageContent that has same id with IncomingMessagecontent");
    outgoingData->makesureTransferable();
    return sendRequest(outgoingData, createMsgHandlerCallback(callback));
}

template<class MessageTrait, class ManagerClient> template<class IncomingMsgContent>
bool QueueingServiceProxy<MessageTrait, ManagerClient>::sendActionRequestSync(const CSMsgContentPtr &outgoingData,
        PayloadProcessCallback<IncomingMsgContent> callback,
        unsigned long maxWaitTimeMs )
{
    assert((outgoingData->operationID() == MessageTrait::template getOperationID<IncomingMsgContent>()) && "Please provide MessageContent that has same id with IncomingMessagecontent");
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
