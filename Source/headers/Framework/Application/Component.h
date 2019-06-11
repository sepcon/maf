#ifndef COMPONENT_H_
#define COMPONENT_H_

#include "headers/Libs/Threading/Interfaces/Queue.h"
#include "headers/Framework/Messaging/Message.h"
#include "headers/Libs/Patterns/Patterns.h"
#include "headers/Framework/Messaging/MessageHandler.h"
#include <memory>
#include <thread>
#include <map>


namespace thaf {

using messaging::MessageBase;


namespace app {
#ifdef MESSAGIN_BY_PRIORITY
    using MessageQueue = threading::PriorityQueue<messaging::MessageBasePtr, messaging::MessageBase::PriorityComp>;
#else
    using MessageQueue = threading::Queue<messaging::MessageBasePtr>;
#endif

class Component : pattern::Unasignable
{
public:
    using ComponentID = std::thread::id;

    static Component *getThreadLocalInstance();
    ComponentID getID() const;
    Component(bool detachedToCurrentThread = true);
    ~Component();
    void start(std::function<void()> entryPointFunc = nullptr);
    void shutdown();
    void postMessage(messaging::MessageBasePtr msg);
    void registerMessageHandler(MessageBase::Type msgType, MessageHandler* handler);
    void registerMessageHandler(MessageBase::Type msgType, MessageHandlerFunc onMessageFunc);

    template<class Msg, typename... Args>
    void postMessage(Args&&... args)
    {
        postMessage(std::make_shared<Msg>(std::forward<Args>(args)...));
    }

    template<class Msg>
    Component& onMessage(MessageHandler* handler)
    {
        registerMessageHandler(MessageBase::idof<Msg>(), handler);
        return *this;
    }

    template <class SpecificMsg >
    Component& onMessage(std::function<void(messaging::CMessagePtr<SpecificMsg>)> f)
    {
        registerMessageHandler(MessageBase::idof<SpecificMsg>(), [f](messaging::CMessageBasePtr msg) {
            auto specifigMsg = std::static_pointer_cast<SpecificMsg>(msg);
            if(specifigMsg) { f(specifigMsg); }
        });
        return *this;
    }

    template<class SignalMsg>
    Component& onSignal(SignalMsgHandlerFunc handler)
    {
        registerMessageHandler(MessageBase::idof<SignalMsg>(), [handler](messaging::CMessageBasePtr){ handler(); });
        return *this;
    }

protected:
    void startMessageLoop();

    std::thread _workerThread;
    MessageQueue _msgQueue;
    std::recursive_mutex _msgHandlerMutex;
    std::map<MessageBase::Type, MessageHandlerFunc> _msgHandlers;
    ComponentID _id;
    bool _detached;
    static thread_local Component* _tlspInstance;
};

}
}
#endif

