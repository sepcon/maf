#pragma once

#include "MessageHandler.h"
#include <maf/patterns/Patterns.h>


#define mc_with_a_message(Message) template<class Message, std::enable_if_t<std::is_base_of_v<MessageBase, Message>, bool> = true>


namespace maf {
namespace threading { class TimerManager; }
namespace messaging {

class Component;
using ComponentRef = std::weak_ptr<Component>;
using ComponentPtr = std::shared_ptr<Component>;
using TimerMgrPtr = std::shared_ptr<threading::TimerManager>;

enum class LaunchMode
{
    Async,
    AttachToCurrentThread
};

class Component final : pattern::UnCopyable, public std::enable_shared_from_this<Component>
{
    struct ComponentImpl;
    Component();
public:
    using BaseMessageHandlerFunc = MessageHandlerFunc<MessageBase>;

    static std::shared_ptr<Component> create();
    static ComponentRef getActiveWeakPtr();
    static std::shared_ptr<Component> getActiveSharedPtr();

    const std::string& name() const;
    void setName(std::string name);
    void run(LaunchMode LaunchMode = LaunchMode::Async, std::function<void()> onEntry = {}, std::function<void()> onExit = {});
    void stop();
    void postMessage(messaging::MessageBasePtr msg);
    void registerMessageHandler(MessageBase::Type msgType, MessageHandler* handler);
    void registerMessageHandler(MessageBase::Type msgType, BaseMessageHandlerFunc onMessageFunc);

    template<class Msg, typename... Args, std::enable_if_t<std::is_constructible_v<Msg, Args...>, bool> = true>
    void postMessage(Args&&... args);

    mc_with_a_message(SpecificMsg)
    Component& onMessage(MessageHandler* handler);

    mc_with_a_message(SpecificMsg)
    Component& onMessage(MessageHandlerFunc<SpecificMsg> f);

    mc_with_a_message(SpecificMsg)
    Component& onSignal(SignalMsgHandlerFunc handler);

    ~Component();

private:
    static void setTLRef(ComponentRef ref);
    static TimerMgrPtr getTimerManager();
    std::string _name;
    ComponentImpl* _pImpl = nullptr;

    friend class Timer;
    friend struct ComponentImpl;
    friend class CompThread;
};


template<class SignalMsg, std::enable_if_t<std::is_base_of_v<MessageBase, SignalMsg>, bool>>
Component &Component::onSignal(SignalMsgHandlerFunc handler)
{
    registerMessageHandler(MessageBase::idof<SignalMsg>(), [handler](messaging::CMessageBasePtr){ handler(); });
    return *this;
}

template<class SpecificMsg, std::enable_if_t<std::is_base_of_v<MessageBase, SpecificMsg>, bool>>
Component &Component::onMessage(MessageHandlerFunc<SpecificMsg> f)
{
    registerMessageHandler(MessageBase::idof<SpecificMsg>(), [f](messaging::CMessageBasePtr& msg) {
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
    postMessage(createMessage<Msg>(std::forward<Args>(args)...));
}

#define mc_tlcomp_invoke(method, ...)          \
if(auto comp = Component::getActiveSharedPtr())\
{                                              \
    comp->method(__VA_ARGS__);                 \
    return true;                               \
}                                              \
else                                           \
{                                              \
    return false;                              \
}

template<class SignalMsg, std::enable_if_t<std::is_base_of_v<MessageBase, SignalMsg>, bool> = true>
bool tlcompOnSignal(SignalMsgHandlerFunc handler)
{
    mc_tlcomp_invoke(onSignal<SignalMsg>, std::move(handler))
}

template<class SpecificMsg, std::enable_if_t<std::is_base_of_v<MessageBase, SpecificMsg>, bool> = true>
bool tlcompOnMessage(MessageHandlerFunc<SpecificMsg> f)
{
    mc_tlcomp_invoke(onMessage<SpecificMsg>, std::move(f))
}

template<class SpecificMsg, std::enable_if_t<std::is_base_of_v<MessageBase, SpecificMsg>, bool> = true>
bool tlcompOnMessage(MessageHandler *handler)
{
    mc_tlcomp_invoke(onMessage<SpecificMsg>, handler)
}

template<class Msg, typename ...Args, std::enable_if_t<std::is_constructible_v<Msg, Args...>, bool> = true>
bool tlcompPostMessage(Args&&... args)
{
    mc_tlcomp_invoke(postMessage<Msg>, std::forward<Args>(args)...)
}

#undef mc_tlcomp_invoke

/// Helper function object for comparing weak_ptr of Component
struct comprefless
{
    bool operator()(const ComponentRef& ref1, const ComponentRef& ref2) const
    {
        return ref1.lock() < ref2.lock();
    }
};

}
}

