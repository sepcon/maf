#ifndef COMPONENT_H_
#define COMPONENT_H_

#include "thaf/Threading/Queue.h"
#include "thaf/Messaging/Message.h"
#include "thaf/Patterns/Patterns.h"
#include "thaf/Messaging/MessageHandler.h"
#include "thaf/Utils/CppExtension/AtomicContainer.h"
#include <memory>
#include <thread>
#include <map>


namespace thaf {

using messaging::MessageBase;


namespace app {
#ifdef MESSAGING_BY_PRIORITY
    using MessageQueue = threading::PriorityQueue<messaging::MessageBasePtr, messaging::MessageBase::PriorityComp>;
#else
    using MessageQueue = threading::Queue<messaging::MessageBasePtr>;
#endif

class Component : pattern::Unasignable
{
public:
    using ComponentID = std::thread::id;
    Component(bool detachFromCurrentThread = true);
    ~Component();

    static Component *getThreadLocalInstance();
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
    using MsgHandlerMap = stl::MutexContainer<std::map<MessageBase::Type, MessageHandlerFunc>>;

    void startMessageLoop();
    std::thread _workerThread;
    MessageQueue _msgQueue;
    MsgHandlerMap _msgHandlers;
    ComponentID _id;
    bool _detached;
    static thread_local Component* _tlspInstance;
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
#endif

