#pragma once

#include "Component.h"

namespace maf {
namespace messaging {

class ExtensibleComponent
{
protected:
    ~ExtensibleComponent() = default;
public:
    using BaseMessageHandlerFunc = Component::BaseMessageHandlerFunc;

    ExtensibleComponent(const std::string& name = "")
    {
        _comp = Component::create();
        _comp->setName(name);
    }

    const std::string& name() const { return _comp->name(); }
    void setName(std::string name);
    void run(LaunchMode launchMode = LaunchMode::Async);
    void stop();
    void postMessage(messaging::MessageBasePtr msg);

    void registerMessageHandler(
        CompMessageBase::Type msgType,
        MessageHandler* handler
        );

    void registerMessageHandler(
        CompMessageBase::Type msgType,
        BaseMessageHandlerFunc onMessageFunc
        );

    template<class Msg, typename... Args,
             std::enable_if_t<std::is_constructible_v<Msg, Args...>,
                              bool> = true
             >
    void postMessage(Args&&... args)
    {
        _comp->postMessage<Msg>(std::forward<Args>(args)...);
    }
    mc_maf_tpl_with_a_message(SpecificMsg)
        ExtensibleComponent& onMessage(MessageHandler* handler)
    {
        _comp->onMessage<SpecificMsg>(handler);
        return *this;
    }
    mc_maf_tpl_with_a_message(SpecificMsg)
        ExtensibleComponent& onMessage(MessageHandlerFunc<SpecificMsg> f)
    {
        _comp->onMessage<SpecificMsg>(f);
        return *this;
    }
    mc_maf_tpl_with_a_message(SpecificMsg)
    ExtensibleComponent& onSignal(SignalMsgHandlerFunc handler)
    {
        _comp->onSignal<SpecificMsg>(handler);
        return *this;
    }
    ComponentPtr component() const
    {
        return _comp;
    }

protected:
    virtual void onEntry() {}
    virtual void onExit() {}

    ComponentPtr _comp;
};

inline void ExtensibleComponent::setName(std::string name)
{ _comp->setName(std::move(name)); }

inline void ExtensibleComponent::run(LaunchMode launchMode)
{
    _comp->run(launchMode, [this]{ onEntry(); }, [this]{onExit(); });
}

inline void ExtensibleComponent::stop()
{
    _comp->stop();
}

inline void ExtensibleComponent::postMessage(MessageBasePtr msg)
{
    _comp->postMessage(std::move(msg));
}

inline void ExtensibleComponent::registerMessageHandler(
    CompMessageBase::Type msgType,
    MessageHandler *handler
    )
{
    _comp->registerMessageHandler(msgType, handler);
}

inline void ExtensibleComponent::registerMessageHandler(
    CompMessageBase::Type msgType,
    ExtensibleComponent::BaseMessageHandlerFunc onMessageFunc
    )
{
    _comp->registerMessageHandler(msgType, std::move(onMessageFunc));
}



}
}
