#pragma once

#include "Component.h"

namespace maf {
namespace messaging {

class ExtensibleComponent
{
public:
    using BaseMessageHandlerFunc = Component::BaseMessageHandlerFunc;

    inline ExtensibleComponent() { _comp = Component::create(); }
    virtual ~ExtensibleComponent() = default;
    inline const std::string& name() const { return _comp->name(); }
    inline void setName(std::string name) { _comp->setName(std::move(name)); }
    inline void run(LaunchMode launchMode = LaunchMode::Async) { _comp->run(launchMode, [this]{ onEntry(); }, [this]{onExit(); }); }
    inline void stop() { _comp->stop(); }
    inline void postMessage(messaging::MessageBasePtr msg) { _comp->postMessage(std::move(msg)); }
    inline void registerMessageHandler(MessageBase::Type msgType, MessageHandler* handler) { _comp->registerMessageHandler(msgType, handler);}
    inline void registerMessageHandler(MessageBase::Type msgType, BaseMessageHandlerFunc onMessageFunc){ _comp->registerMessageHandler(msgType, std::move(onMessageFunc));}
    template<class Msg, typename... Args, std::enable_if_t<std::is_constructible_v<Msg, Args...>, bool> = true>
    inline void postMessage(Args&&... args){ _comp->postMessage<Msg>(std::forward<Args>(args)...);}
    mc_maf_tpl_with_a_message(SpecificMsg)
        ExtensibleComponent& onMessage(MessageHandler* handler) { _comp->onMessage<SpecificMsg>(handler); return *this;}
    mc_maf_tpl_with_a_message(SpecificMsg)
        ExtensibleComponent& onMessage(MessageHandlerFunc<SpecificMsg> f) {_comp->onMessage<SpecificMsg>(f); return *this;}
    mc_maf_tpl_with_a_message(SpecificMsg)
    ExtensibleComponent& onSignal(SignalMsgHandlerFunc handler) { _comp->onSignal<SpecificMsg>(handler); return *this;}

protected:
    virtual void onEntry() {}
    virtual void onExit() {}

    ComponentPtr _comp;
};



}
}
