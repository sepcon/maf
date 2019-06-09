#include "headers/Application/Framework/Component.h"
#include "headers/Application/Framework/Messages.h"
#include "headers/Threading/Interfaces/ThreadPoolFactory.h"


namespace thaf {
namespace app {

using namespace Messaging;
Component* Component::_appMainComponent = nullptr;
thread_local Component* Component::_tlspInstance = nullptr;

Component::Component(bool isMainComponent, bool detachedToCurrentThread) : _detached(detachedToCurrentThread)
{
    if(isMainComponent)
    {
        _appMainComponent = this;
    }
}

void Component::postMessage(MessagePtr msg)
{
    _msgQueue.push(std::move(msg));
}

void Component::registerMessageHandler(Message::Type msgType, MessageHandler *handler)
{
    if(handler)
    {
        registerMessageHandler(msgType, [handler](const std::shared_ptr<Message>& msg) { handler->onMessage(msg);});
    }
}

void Component::registerMessageHandler(Message::Type msgType, MessageHandlerFunc onMessageFunc)
{
    if(onMessageFunc)
    {
        std::lock_guard<std::recursive_mutex> lock(_msgHandlerMutex);
        _msgHandlers.insert(std::make_pair(msgType, onMessageFunc));
    }
}

void Component::start(MessageHandlerFunc entryPointFunc)
{
    if(entryPointFunc)
    {
        match<StartupMessage>(entryPointFunc);
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

Component *Component::getMainComponent()
{
    return _appMainComponent;
}

void Component::startMessageLoop()
{
    _tlspInstance = this;
    std::shared_ptr<Message> msg;
    while(_msgQueue.wait(msg))
    {
        if(!msg)
        {
            continue;
        }
        else
        {
            std::lock_guard<std::recursive_mutex> lock(_msgHandlerMutex);
            auto handlers = _msgHandlers.equal_range(Message::idof(*msg));
            for(auto itHandler = handlers.first; itHandler != handlers.second; ++itHandler)
            {
                auto& onMsgFunc = itHandler->second;
                if(onMsgFunc)
                {
                    onMsgFunc(msg);
                }
            }
        }
    }
}

}
}
