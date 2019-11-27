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

    inline ExtensibleComponent(const std::string& name = "")
    {
        _comp = Component::create();
        _comp->setName(name);
    }

    inline const std::string& name() const { return _comp->name(); }
    inline void setName(std::string name);
    inline void run(LaunchMode launchMode = LaunchMode::Async);
    inline void stop();
    inline void postMessage(messaging::MessageBasePtr msg);

    inline void registerMessageHandler(
        CompMessageBase::Type msgType,
        MessageHandler* handler
        );

    inline void registerMessageHandler(
        CompMessageBase::Type msgType,
        BaseMessageHandlerFunc onMessageFunc
        );

    template<class Msg, typename... Args,
             std::enable_if_t<std::is_constructible_v<Msg, Args...>,
                              bool> = true
             >
    inline void postMessage(Args&&... args)
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
    virtual void onEntry() = 0;
    virtual void onExit() {}

    ComponentPtr _comp;
};

void ExtensibleComponent::setName(std::string name)
{ _comp->setName(std::move(name)); }

void ExtensibleComponent::run(LaunchMode launchMode)
{
    _comp->run(launchMode, [this]{ onEntry(); }, [this]{onExit(); });
}

void ExtensibleComponent::stop()
{
    _comp->stop();
}

void ExtensibleComponent::postMessage(MessageBasePtr msg)
{
    _comp->postMessage(std::move(msg));
}

void ExtensibleComponent::registerMessageHandler(
    CompMessageBase::Type msgType,
    MessageHandler *handler
    )
{
    _comp->registerMessageHandler(msgType, handler);
}

void ExtensibleComponent::registerMessageHandler(
    CompMessageBase::Type msgType,
    ExtensibleComponent::BaseMessageHandlerFunc onMessageFunc
    )
{
    _comp->registerMessageHandler(msgType, std::move(onMessageFunc));
}



}
}
