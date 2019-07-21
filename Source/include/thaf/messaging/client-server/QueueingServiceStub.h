#pragma once

#include "thaf/messaging/client-server/ServiceStubBase.h"
#include "thaf/messaging/Component.h"
#include "thaf/messaging/BasicMessages.h"

namespace thaf {
namespace messaging {


template<class MessageTrait, class ControllingServer>
struct ClientRequestMessage : public ExternalMessage
{
    using MyRequestKeeper = RequestKeeper<MessageTrait>;
    using MyRequestKeeperPtr = std::shared_ptr<MyRequestKeeper>;
public:
    ClientRequestMessage(MyRequestKeeperPtr clp): _requestKeeper(std::move(clp)){}
    MyRequestKeeperPtr getRequestKeeper() { return _requestKeeper; }
    template<class CSMessageContentSpecific>
    std::shared_ptr<CSMessageContentSpecific> getRequestData() const noexcept {
        return _requestKeeper->template getRequestContent<CSMessageContentSpecific>();
    }

private:
    MyRequestKeeperPtr _requestKeeper;
};

/**
 * @brief QueueingServiceStub class provides a generic interface of a ServiceStub that is tight coupling with
 * a messaging::Component, to help handling Service Message in a queueing maner to prevent issue of data races
 * coping with multithreading application.
 * @class MessageTrait: must provide interfaces of translating specific type messages to CSMessage and vice versa
 * @class ControllingServer: must satisfy be a ServerInterface and is a pattern::SingletonObject (see patterns.h)
 */
template <class MessageTrait, class ControllingServer>
class QueueingServiceStub : public ServiceStubBase, public ServiceStubHandlerInterface
{
    using MyRequestKeeper = RequestKeeper<MessageTrait>;
    using MyRequestMessage = ClientRequestMessage<MessageTrait, ControllingServer>;
    using MyRequestMessagePtr = std::shared_ptr<MyRequestMessage>;
    using MyRequestKeeperPtr = std::shared_ptr<MyRequestKeeper>;
    using MyType = QueueingServiceStub<MessageTrait, ControllingServer>;
    using MyRef = std::shared_ptr<MyType>;
public:
    static MyRef createStub(ServiceID sid);
    template<class CSMessageContentSpecific>
    bool sendStatusUpdate(const std::shared_ptr<CSMessageContentSpecific>& msgContent);
//    ~QueueingServiceStub();

protected:
    QueueingServiceStub(ServiceID sid);
    void onClientRequest(const std::shared_ptr<RequestKeeperBase>& requestKeeper) override;
    ComponentRef _compref;
};

template <class MessageTrait, class ControllingServer> template<class CSMessageContentSpecific>
bool QueueingServiceStub<MessageTrait, ControllingServer>::sendStatusUpdate(const std::shared_ptr<CSMessageContentSpecific> &msgContent)
{
    auto csMsgContent = MessageTrait::template translate(msgContent);

    CSMessagePtr csMsg = createCSMessage(
        this->serviceID(),
        MessageTrait::template getOperationID<CSMessageContentSpecific>(),
        OpCode::Register
        );

    csMsg->setContent(std::move(csMsgContent));
    return ServiceStubBase::sendStatusUpdate(csMsg);
}

template<class MessageTrait, class ControllingServer>
typename QueueingServiceStub<MessageTrait, ControllingServer>::MyRef QueueingServiceStub<MessageTrait, ControllingServer>::createStub(ServiceID sid)
{
    auto serviceProvider = ControllingServer::instance().getServiceProvider(sid);
    if(!serviceProvider)
    {
        serviceProvider.reset(new MyType(sid));
        ControllingServer::instance().registerServiceProvider(serviceProvider);
    }
    auto stub = std::static_pointer_cast<MyType>(serviceProvider);
    return stub;
}

//template<class MessageTrait, class ControllingServer>
//QueueingServiceStub<MessageTrait, ControllingServer>::~QueueingServiceStub()
//{
//}

template<class MessageTrait, class ControllingServer>
QueueingServiceStub<MessageTrait, ControllingServer>::QueueingServiceStub(ServiceID sid) : ServiceStubBase(sid, &ControllingServer::instance(), this)
{
    _compref = Component::getComponentRef();
}

template<class MessageTrait, class ControllingServer>
void QueueingServiceStub<MessageTrait, ControllingServer>::onClientRequest(const std::shared_ptr<RequestKeeperBase> &requestKeeper)
{
    auto lock(_compref->pa_lock());
    if(_compref->get())
    {
        _compref->get()->postMessage<MyRequestMessage>(std::static_pointer_cast<MyRequestKeeper>(requestKeeper) );
    }
}

}
}
