#include <maf/messaging/Component.h>
#include <maf/threading/Lockable.h>
#include <maf/threading/Queue.h>
#include <maf/messaging/BasicMessages.h>
#include <maf/logging/Logger.h>
#include <memory>
#include <map>


namespace maf { using logging::Logger;
namespace messaging {

using MessageQueue = threading::Queue<std::any>;
using HandlerMap = threading::Lockable<
                            std::map<ComponentMessageID,
                            GenericMsgHandlerFunction
                            >>;
struct ComponentDataPrv
{
    std::string         name;
    MessageQueue        msgq;
    HandlerMap          handlers;
};


static thread_local ComponentRef _tlwpInstance;

Component::Component()
    : d_{ new ComponentDataPrv }
{
    onMessage<CallbackExcMsg>([](CallbackExcMsg msg){
        msg.execute();
    });
}

Component::~Component()
{
    stop();
}

std::shared_ptr<Component> Component::create()
{
    return std::shared_ptr<Component>{ new Component, [](auto comp){ delete comp; }};
}

const std::string &Component::name() const
{
    return d_->name;
}

void Component::setName(std::string name)
{
    d_->name = std::move(name);
}

void Component::run(
    std::function<void()> onEntry,
    std::function<void()> onExit
    )
{
    _tlwpInstance = weak_from_this();
    if(onEntry) { onEntry(); }

    ComponentMessage msg;
    while(d_->msgq.wait(msg))
    {
        GenericMsgHandlerFunction handlerFunc;
        {
            std::lock_guard lock(d_->handlers);
            auto itHandler = d_->handlers->find(msg.type());
            if(itHandler != d_->handlers->end())
            {
                handlerFunc = itHandler->second;
            }
        }

        if(handlerFunc)
        {
            try
            {
                handlerFunc(std::move(msg));
            }
            catch(const std::exception& e)
            {
                Logger::error(
                    "Exception occurred while executing "
                    "messageHandler function: " ,  e.what()
                    );
            }
            catch(...)
            {
                Logger::error(
                    "Unknown exception occurred while executing"
                              " messageHandler function: "
                              );
            }
        }
        else
        {
            Logger::warn(
                "There's no handler for message " ,
                msg.type().name()
                );
        }
    }

    if(onExit) { onExit(); }
}

std::future<void> Component::runAsync(std::function<void ()> onEntry, std::function<void ()> onExit)
{
    return std::async(
                std::launch::async,
                std::bind(&Component::run, this, std::move(onEntry), std::move(onExit))
                );
}

void Component::stop()
{
    d_->msgq.close();
}

bool Component::post(ComponentMessage&& msg)
{
    try
    {
        d_->msgq.push(std::move(msg));
        return true;
    }
    catch(const std::bad_alloc& ba)
    {
        Logger::error("Queue overflow: " ,  ba.what());
    }
    catch(const std::exception& e)
    {
        Logger::error("Exception occurred when pushing data to queue: " ,  e.what());
    }
    catch(...)
    {
        Logger::error("Unkown exception occurred when pushing data to queue!");
    }
    return false;
}

void Component::registerMessageHandler(
    ComponentMessageID msgid,
    std::weak_ptr<ComponentMessageHandler> handler
    )
{
    auto handlerFunction = [handler = std::move(handler)](ComponentMessage msg)
    {
        if(auto handlerPtr = handler.lock())
        {
            handlerPtr->onMessage(std::move(msg));
        }
    };

    registerMessageHandler(msgid, std::move(handlerFunction));
}

void Component::registerMessageHandler(
    ComponentMessageID msgid,
    GenericMsgHandlerFunction onMessageFunc
    )
{
    if(onMessageFunc)
    {
        d_->handlers.atomic()->emplace(
            std::move(msgid), std::move(onMessageFunc)
            );
    }
}

bool Component::unregisterHandler(ComponentMessageID msgid)
{
    std::lock_guard lock(d_->handlers);
    return d_->handlers->erase(msgid) > 0;
}

void Component::logError(const std::string &info)
{
    Logger::error(info);
}

std::shared_ptr<Component> RunningComponent::shared()
{
    return _tlwpInstance.lock();
}

std::weak_ptr<Component> RunningComponent::weak()
{
    return _tlwpInstance;
}

bool RunningComponent::stop()
{
    if(auto comp = shared())
    {
        comp->stop();
        return false;
    }
    return false;
}

bool RunningComponent::post(ComponentMessage &&msg)
{
    if(auto comp = shared())
    {
        return comp->post(std::move(msg));
    }
    return false;
}


}
}
