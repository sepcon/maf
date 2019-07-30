#pragma once

#include "thaf/utils/cppextension/SyncObject.h"
#include "thaf/threading/TimerManager.h"
#include "thaf/patterns/Patterns.h"
#include "MessageQueue.h"
#include "MessageHandler.h"
#include <memory>
#include <thread>
#include <map>


namespace thaf {
namespace messaging {

class Component;
using ComponentSyncPtr = nstl::SyncObject<Component*>;
using ComponentRef = std::shared_ptr<ComponentSyncPtr>;
using TimerMgrPtr = std::shared_ptr<threading::TimerManager>;

class Component : pattern::Unasignable
{
public:
    using ComponentID = std::thread::id;
    Component(bool detachFromCurrentThread = true);
    ~Component();

    static ComponentRef getComponentRef();

    ComponentID getID() const;

    void start(std::function<void()> entryPointFunc = nullptr);
    void shutdown();
    void postMessage(messaging::MessageBasePtr msg);
    void registerMessageHandler(MessageBase::Type msgType, MessageHandler* handler);
    void registerMessageHandler(MessageBase::Type msgType, MessageHandlerFunc onMessageFunc);

    template<class Msg, typename... Args, std::enable_if_t<std::is_constructible_v<Msg, Args...>, bool> = true>
    void postMessage(Args&&... args);

    template<class SpecificMsg, std::enable_if_t<std::is_base_of_v<MessageBase, SpecificMsg>, bool> = true>
    Component& onMessage(MessageHandler* handler);

    template<class SpecificMsg, std::enable_if_t<std::is_base_of_v<MessageBase, SpecificMsg>, bool> = true>
    Component& onMessage(std::function<void(messaging::CMessagePtr<SpecificMsg>&)> f);

    template<class SpecificMsg, std::enable_if_t<std::is_base_of_v<MessageBase, SpecificMsg>, bool> = true>
    Component& onSignal(SignalMsgHandlerFunc handler);

protected:
    using MsgHandlerMap = nstl::SyncObject<std::map<MessageBase::Type, MessageHandlerFunc>>;
    static TimerMgrPtr getTimeManager();
    void startMessageLoop();
    std::thread _workerThread;
    MessageQueue _msgQueue;
    MsgHandlerMap _msgHandlers;
    bool _detached;
    TimerMgrPtr _timerMgr;

    ComponentRef _myPtr;
    static thread_local Component* _tlspInstance;

    friend class Timer;
};

template<class SignalMsg, std::enable_if_t<std::is_base_of_v<MessageBase, SignalMsg>, bool>>
Component &Component::onSignal(SignalMsgHandlerFunc handler)
{
    registerMessageHandler(MessageBase::idof<SignalMsg>(), [handler](messaging::CMessageBasePtr){ handler(); });
    return *this;
}

template<class SpecificMsg, std::enable_if_t<std::is_base_of_v<MessageBase, SpecificMsg>, bool>>
Component &Component::onMessage(std::function<void(messaging::CMessagePtr<SpecificMsg>&) > f)
{
    registerMessageHandler(MessageBase::idof<SpecificMsg>(), [f](messaging::CMessageBasePtr msg) {
        auto specifigMsg = std::static_pointer_cast<SpecificMsg>(msg);
        if(specifigMsg) { f(specifigMsg); }
    });
    return *this;
}

template<class SpecificMsg, std::enable_if_t<std::is_base_of_v<MessageBase, SpecificMsg>, bool>>
Component &Component::onMessage(MessageHandler *handler)
{
    registerMessageHandler(MessageBase::idof<SpecificMsg>(), handler);
    return *this;
}

template<class Msg, typename ...Args, std::enable_if_t<std::is_constructible_v<Msg, Args...>, bool>>
void Component::postMessage(Args&&... args)
{
    postMessage(std::make_shared<Msg>(std::forward<Args>(args)...));
}

}
}

