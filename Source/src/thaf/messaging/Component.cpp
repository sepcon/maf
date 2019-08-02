#include "thaf/messaging/Component.h"
#include "thaf/messaging/BasicMessages.h"
#include "thaf/utils/debugging/Debug.h"


namespace thaf {
namespace messaging {

thread_local Component* Component::_tlspInstance = nullptr;

Component::Component(bool detachFromCurrentThread) :
    _detached(detachFromCurrentThread)
{
    onMessage<TimeoutMessage>([](CMessagePtr<TimeoutMessage> msg) {
        msg->execute();
    });
    onMessage<CallbackExcMsg>([](CMessagePtr<CallbackExcMsg> msg) {
        msg->execute();
    });
}

void Component::postMessage(MessageBasePtr msg)
{
    try
    {
        _msgQueue.push(std::move(msg));
    }
    catch(const std::bad_alloc& ba)
    {
        thafErr("Queue overflow: " << ba.what());
    }
    catch(const std::exception& e)
    {
        thafErr("Exception occurred when pushing data to queue: " << e.what());
    }
    catch(...)
    {
        thafErr("Unkown exception occurred when pushing data to queue!");
    }
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
        auto lock = _msgHandlers.a_lock();
        _msgHandlers->insert(std::make_pair(msgType, onMessageFunc));
    }
}

TimerMgrPtr Component::getTimeManager()
{
    if(_tlspInstance)
    {
        if(!_tlspInstance->_timerMgr)
        {
            _tlspInstance->_timerMgr = std::make_shared<threading::TimerManager>();
        }
        return _tlspInstance->_timerMgr; // at this point, the timer object will never be destroyed if still have someone holding its reference(shared_ptr)
    }
    else
    {
        return {};
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
    if(std::this_thread::get_id() != _workerThread.get_id())
    {
        if(_workerThread.joinable())
        {
            _workerThread.join();
        }
    }
}

Component::~Component()
{
    if (_timerMgr)
    {
        _timerMgr.reset();
    }
    shutdown();

    if(_myPtr)
    {
        auto lock(_myPtr->pa_lock());
        _myPtr->reset(); //To tell others who are holding this reference that the component is already destroyed!
    }
}

ComponentRef Component::getComponentRef()
{
    if(_tlspInstance)
    {
        if(!_tlspInstance->_myPtr)
        {
            _tlspInstance->_myPtr = std::make_shared<ComponentPtrSync>(_tlspInstance);
        }
        return _tlspInstance->_myPtr;
    }
    else
    {
        return {};
    }
}

const Component::ComponentName &Component::name() const
{
    return _name;
}

void Component::setName(Component::ComponentName name)
{
    _name = std::move(name);
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
            MessageHandlerFunc handlerFunc;
            {
                auto lock = _msgHandlers.a_lock();
                auto itHandler = _msgHandlers->find(MessageBase::idof(*msg));
                if(itHandler != _msgHandlers->end())
                {
                    handlerFunc = itHandler->second;
                }
            }

            if(handlerFunc)
            {
                try
                {
                    if(handlerFunc)
                    {
                        handlerFunc(msg);
                    }
                }
                catch(const std::exception& e)
                {
                    thafErr("Exception occurred while executing messageHandler function: " << e.what());
                }
                catch(...)
                {
                    thafErr("Unknown exception occurred while executing messageHandler function: ");
                }
            }
            else
            {
                thafWarn("There's no handler for message " << MessageBase::idof(*msg).name());
            }
        }
    }
}

}
}
