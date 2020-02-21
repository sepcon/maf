#pragma once

#include "MessageHandler.h"
#include <maf/patterns/Patterns.h>
#include <maf/export/MafExport_global.h>


#define mc_maf_tpl_with_a_message(Message) template<class Message, std::enable_if_t<std::is_base_of_v<CompMessageBase, Message>, bool> = true>


namespace maf {
namespace messaging {

class TimerManager;
class Component;
struct ComponentImpl;

using ComponentRef    = std::weak_ptr<Component>;
using ComponentPtr    = std::shared_ptr<Component>;
using TimerMgrPtr     = std::shared_ptr<TimerManager>;

enum class LaunchMode
{
    Async,
    AttachToCurrentThread
};


class Component final : pattern::Unasignable, public std::enable_shared_from_this<Component>
{
    std::string _name;
    std::unique_ptr<ComponentImpl> _pImpl;
    MAF_EXPORT Component();
public:
    using BaseMessageHandlerFunc = MessageHandlerFunc<CompMessageBase>;

    MAF_EXPORT static std::shared_ptr<Component> create();
    MAF_EXPORT static ComponentRef getActiveWeakPtr();
    MAF_EXPORT static std::shared_ptr<Component> getActiveSharedPtr();

    MAF_EXPORT const std::string& name() const;
    MAF_EXPORT void setName(std::string name);
    MAF_EXPORT void run(LaunchMode LaunchMode = LaunchMode::Async, std::function<void()> onEntry = {}, std::function<void()> onExit = {});
    MAF_EXPORT void stop();
    MAF_EXPORT void postMessage(messaging::MessageBasePtr msg);
    MAF_EXPORT void registerMessageHandler(CompMessageBase::Type msgType, MessageHandler* handler);
    MAF_EXPORT void registerMessageHandler(CompMessageBase::Type msgType, BaseMessageHandlerFunc onMessageFunc);

    template<class Msg, typename... Args, std::enable_if_t<std::is_constructible_v<Msg, Args...>, bool> = true>
    void postMessage(Args&&... args);

    mc_maf_tpl_with_a_message(SpecificMsg)
    Component& onMessage(MessageHandler* handler);

    mc_maf_tpl_with_a_message(SpecificMsg)
    Component& onMessage(MessageHandlerFunc<SpecificMsg> f);

    mc_maf_tpl_with_a_message(SpecificMsg)
    Component& onSignal(SignalMsgHandlerFunc handler);

    MAF_EXPORT ~Component();

private:
    static void setTLRef(ComponentRef ref);
    static TimerMgrPtr getTimerManager();

    friend class TimerImpl;
    friend struct ComponentImpl;
    friend class CompThread;
};


template<class SignalMsg, std::enable_if_t<std::is_base_of_v<CompMessageBase, SignalMsg>, bool>>
Component &Component::onSignal(SignalMsgHandlerFunc handler)
{
    registerMessageHandler(msgID<SignalMsg>(), [handler](messaging::CMessageBasePtr){ handler(); });
    return *this;
}

template<class SpecificMsg, std::enable_if_t<std::is_base_of_v<CompMessageBase, SpecificMsg>, bool>>
Component &Component::onMessage(MessageHandlerFunc<SpecificMsg> f)
{
    registerMessageHandler(msgID<SpecificMsg>(), [f](messaging::CMessageBasePtr& msg) {
        auto specifigMsg = std::static_pointer_cast<SpecificMsg>(msg);
        if(specifigMsg) { f(specifigMsg); }
    });
    return *this;
}

template<class SpecificMsg, std::enable_if_t<std::is_base_of_v<CompMessageBase, SpecificMsg>, bool>>
Component &Component::onMessage(MessageHandler *handler)
{
    registerMessageHandler(msgID<SpecificMsg>(), handler);
    return *this;
}

template<class Msg, typename ...Args, std::enable_if_t<std::is_constructible_v<Msg, Args...>, bool>>
void Component::postMessage(Args&&... args)
{
    postMessage(makeCompMessage<Msg>(std::forward<Args>(args)...));
}

#define mc_maf_tlcomp_invoke(method, ...)      \
if(auto comp = Component::getActiveSharedPtr())\
{                                              \
    comp->method(__VA_ARGS__);                 \
    return true;                               \
}                                              \
else                                           \
{                                              \
    return false;                              \
}

template<class SignalMsg, std::enable_if_t<std::is_base_of_v<CompMessageBase, SignalMsg>, bool> = true>
bool tlcompOnSignal(SignalMsgHandlerFunc handler)
{
    mc_maf_tlcomp_invoke(onSignal<SignalMsg>, std::move(handler))
}

template<class SpecificMsg, std::enable_if_t<std::is_base_of_v<CompMessageBase, SpecificMsg>, bool> = true>
bool tlcompOnMessage(MessageHandlerFunc<SpecificMsg> f)
{
    mc_maf_tlcomp_invoke(onMessage<SpecificMsg>, std::move(f))
}

template<class SpecificMsg, std::enable_if_t<std::is_base_of_v<CompMessageBase, SpecificMsg>, bool> = true>
bool tlcompOnMessage(MessageHandler *handler)
{
    mc_maf_tlcomp_invoke(onMessage<SpecificMsg>, handler)
}

template<class Msg, typename ...Args, std::enable_if_t<std::is_constructible_v<Msg, Args...>, bool> = true>
bool tlcompPostMessage(Args&&... args)
{
    mc_maf_tlcomp_invoke(postMessage<Msg>, std::forward<Args>(args)...)
}

#undef mc_maf_tlcomp_invoke

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

