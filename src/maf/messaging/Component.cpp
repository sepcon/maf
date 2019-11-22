#include <maf/messaging/MessageQueue.h>
#include <maf/messaging/Component.h>
#include <maf/threading/Lockable.h>
#include <maf/messaging/BasicMessages.h>
#include <maf/logging/Logger.h>
#include "TimerManager.h"
#include <thread>
#include <memory>
#include <map>


namespace maf { using logging::Logger;
namespace messaging {

using MsgHandlerMap = threading::Lockable<std::map<CompMessageBase::Type, MessageHandlerFunc<CompMessageBase>>>;
using TimerMgrPtr = std::shared_ptr<TimerManager>;

static thread_local ComponentRef _tlwpInstance;

struct ComponentImpl
{
    using BaseMessageHandlerFunc = Component::BaseMessageHandlerFunc;
    std::unique_ptr<std::thread> _workerThread;
    MessageQueue _msgQueue;
    MsgHandlerMap _msgHandlers;
    TimerMgrPtr _timerMgr;

    ComponentImpl();
    ~ComponentImpl();
    void run(ComponentRef compref, LaunchMode LaunchMode, std::function<void()> onEntry, std::function<void()> onExit);
    void stop();
    void postMessage(MessageBasePtr msg);
    void registerMessageHandler(CompMessageBase::Type msgType, MessageHandler* handler);
    void registerMessageHandler(CompMessageBase::Type msgType, BaseMessageHandlerFunc onMessageFunc);
    TimerMgrPtr getTimerManager();
    void startMessageLoop(ComponentRef compref, std::function<void()> onEntry, std::function<void()> onExit);
};


ComponentImpl::ComponentImpl()
{
    registerMessageHandler(msgID<TimeoutMessage>(), [](const auto& msg) {
        auto timeoutMsg = std::static_pointer_cast<TimeoutMessage>(msg);
        timeoutMsg->execute();
    });
    registerMessageHandler(msgID<CallbackExcMsg>(), [](const auto& msg) {
        auto cbExcMsg = std::static_pointer_cast<CallbackExcMsg>(msg);
        cbExcMsg->execute();
    });
}

ComponentImpl::~ComponentImpl()
{
    stop();
}

void ComponentImpl::run(ComponentRef compref, LaunchMode LaunchMode, std::function<void()> onEntry, std::function<void()> onExit)
{
    if(LaunchMode == LaunchMode::Async)
    {
        _workerThread = std::make_unique<std::thread>(
            [this, compref, onEntry, onExit] {
            this->startMessageLoop(compref, onEntry, onExit);
        });
    }
    else
    {
        this->startMessageLoop(compref, onEntry, onExit);
    }
}

void ComponentImpl::stop()
{
    _msgQueue.close();
    if (_timerMgr) { _timerMgr->stop(); _timerMgr.reset(); }
    if (_workerThread && (std::this_thread::get_id() != _workerThread->get_id()))
    {
        if (_workerThread->joinable())
        {
            _workerThread->join();
        }
    }
}

void ComponentImpl::postMessage(MessageBasePtr msg)
{
    try
    {
        _msgQueue.push(std::move(msg));
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
}

void ComponentImpl::registerMessageHandler(CompMessageBase::Type msgType, MessageHandler *handler)
{
    if(handler)
    {
        registerMessageHandler(msgType, [handler](const std::shared_ptr<CompMessageBase>& msg) { handler->onMessage(msg);});
    }
}

void ComponentImpl::registerMessageHandler(CompMessageBase::Type msgType, BaseMessageHandlerFunc onMessageFunc)
{
    if(onMessageFunc)
    {
        _msgHandlers.atomic()->insert(std::make_pair(msgType, onMessageFunc));
    }
}

TimerMgrPtr ComponentImpl::getTimerManager()
{
    if(!_timerMgr)
    {
        _timerMgr = std::make_shared<TimerManager>();
    }
    return _timerMgr; // at this point, the timer object will never be destroyed if still have someone holding its reference(shared_ptr)
}

void ComponentImpl::startMessageLoop(ComponentRef compref, std::function<void()> onEntry, std::function<void()> onExit)
{
    _tlwpInstance = std::move(compref);
    if(onEntry)
    {
        if(auto component = _tlwpInstance.lock(); component) onEntry();
    }

    MessageBasePtr msg;
    while(_msgQueue.wait(msg))
    {
        if(!msg)
        {
            Logger::error("Got nullptr message");
            continue;
        }
        else
        {
            BaseMessageHandlerFunc handlerFunc;
            {
                std::lock_guard lock(_msgHandlers);
                auto itHandler = _msgHandlers->find(msgID(*msg));
                if(itHandler != _msgHandlers->end())
                {
                    handlerFunc = itHandler->second;
                }
            }

            if(handlerFunc)
            {
                try
                {
                    handlerFunc(msg);
                }
                catch(const std::exception& e)
                {
                    Logger::error("Exception occurred while executing messageHandler function: " ,  e.what());
                }
                catch(...)
                {
                    Logger::error("Unknown exception occurred while executing messageHandler function: ");
                }
            }
            else
            {
                Logger::warn("There's no handler for message " ,  msgID(*msg).name());
            }
        }
    }

    if(onExit)
    {
        if(auto component = _tlwpInstance.lock()) onExit();
    }
}

Component::Component() : _pImpl{ new ComponentImpl } {}
Component::~Component() = default;
std::shared_ptr<Component> Component::create() { return std::shared_ptr<Component>{ new Component};}
const std::string &Component::name() const { return _name; }
void Component::setName(std::string name) { _name = std::move(name); }
void Component::run(LaunchMode LaunchMode, std::function<void()> onEntry, std::function<void()> onExit) { _pImpl->run(weak_from_this(), LaunchMode, onEntry, onExit); }
void Component::stop(){ _pImpl->stop();}
void Component::postMessage(MessageBasePtr msg) { _pImpl->postMessage(msg); }
void Component::registerMessageHandler(CompMessageBase::Type msgType, MessageHandler *handler) { _pImpl->registerMessageHandler(msgType, handler); }
void Component::registerMessageHandler(CompMessageBase::Type msgType, BaseMessageHandlerFunc onMessageFunc){ _pImpl->registerMessageHandler(msgType, std::move(onMessageFunc)); }
ComponentRef Component::getActiveWeakPtr() { return _tlwpInstance; }
std::shared_ptr<Component> Component::getActiveSharedPtr(){ return _tlwpInstance.lock(); }
void Component::setTLRef(ComponentRef ref) { if(!getActiveSharedPtr()) _tlwpInstance = std::move(ref);}
TimerMgrPtr Component::getTimerManager()
{
    auto spInstance = _tlwpInstance.lock();
    if(spInstance)
    {
        return spInstance->_pImpl->getTimerManager();
    }
    else
    {
        return {};
    }
}

}
}
