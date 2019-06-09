#ifndef COMPONENT_H_
#define COMPONENT_H_

#include "headers/Threading/Interfaces/Queue.h"
#include "headers/Messaging/Message.h"
#include "MessageHandler.h"
#include <memory>
#include <thread>
#include <map>


namespace thaf {

using Messaging::Message;

namespace app {

class Component
{
public:
    Component(bool isMainComponent = false, bool detachedToCurrentThread = true);
    virtual ~Component();
    static Component *getThreadLocalInstance();
    static Component* getMainComponent();
    virtual void start(MessageHandlerFunc entryPointFunc = nullptr);
    virtual void shutdown();
    virtual void postMessage(Messaging::MessagePtr msg);
    virtual void registerMessageHandler(Message::Type msgType, MessageHandler* handler);
    virtual void registerMessageHandler(Message::Type msgType, MessageHandlerFunc onMessageFunc);

    template <typename DerivedComponent>
    static DerivedComponent* getThreadLocalInstance()
    {
		return static_cast<DerivedComponent*>(_tlspInstance);
        /*if(typeid (DerivedComponent) == typeid (*_tlspInstance))
        {
            return static_cast<DerivedComponent*>(_tlspInstance);
        }
        else
        {
            return nullptr;
        }*/
    }

    template<class Msg, typename... Args>
    void postMessage(Args&&... args)
    {
        postMessage(std::make_shared<Msg>(std::forward<Args>(args)...));
    }

    template<class Msg, class Handler>
    Component& match(Handler handler)
    {
        registerMessageHandler(Message::idof<Msg>(), handler);
        return *this;
    }

protected:
    void startMessageLoop();

private:
    Component(const Component&) = delete;
    Component(Component&&) = delete;
    Component& operator=(const Component&) = delete;
    Component& operator=(Component&&) = delete;

    std::thread _workerThread;
    Threading::Queue<Messaging::MessagePtr> _msgQueue;
    std::recursive_mutex _msgHandlerMutex;
    std::multimap<Message::Type, MessageHandlerFunc> _msgHandlers;
    bool _detached;
    static Component* _appMainComponent;
    static thread_local Component* _tlspInstance;
};

}
}
#endif

