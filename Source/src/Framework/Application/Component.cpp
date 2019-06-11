#include "headers/Framework/Application/Component.h"
#include "headers/Framework/Application/Messages.h"


namespace thaf {
namespace app {

using namespace messaging;

thread_local Component* Component::_tlspInstance = nullptr;

Component::Component(bool detachedToCurrentThread) :
    _detached(detachedToCurrentThread)
{

}

void Component::postMessage(MessageBasePtr msg)
{
    _msgQueue.push(std::move(msg));
}

void Component::registerMessageHandler(MessageBase::Type msgType, MessageHandler *handler)
{
    if(handler)
    {
        registerMessageHandler(msgType, [handler](const std::shared_ptr<MessageBase>& msg) { handler->onMessage(msg);});
    }
}

void Component::registerMessageHandler(MessageBase::Type msgType, MessageHandlerFunc onMessageFunc)
{
    if(onMessageFunc)
    {
        std::lock_guard<std::recursive_mutex> lock(_msgHandlerMutex);
        _msgHandlers.insert(std::make_pair(msgType, onMessageFunc));
    }
}

void Component::start(std::function<void()> entryPointFunc)
{
    if(entryPointFunc)
    {
        onSignal<StartupMessage>(entryPointFunc);
        postMessage<StartupMessage>();
    }

    if(_detached)
    {
        _workerThread = std::thread {[this] {
            this->startMessageLoop();
        }};
    }
    else
    {
        this->startMessageLoop();
    }
}

void Component::shutdown()
{
    _msgQueue.close();
    if(_workerThread.joinable())
    {
        _workerThread.join();
    }
}

Component::~Component()
{
    shutdown();
}

Component *Component::getThreadLocalInstance()
{
    return _tlspInstance;
}

Component::ComponentID Component::getID() const
{
    return _id;
}

void Component::startMessageLoop()
{
    _tlspInstance = this;
    MessageBasePtr msg;
    while(_msgQueue.wait(msg))
    {
        if(!msg)
        {
            continue;
        }
        else
        {
            std::lock_guard<std::recursive_mutex> lock(_msgHandlerMutex);
            auto itHandler = _msgHandlers.find(MessageBase::idof(*msg));
            if(itHandler != _msgHandlers.end())
            {
                auto& handlerFunction = itHandler->second;
                handlerFunction(msg);
            }
        }
    }
}

}
}
