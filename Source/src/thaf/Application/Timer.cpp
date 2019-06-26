#include "thaf/Application/Timer.h"
#include "thaf/Threading/TimerManager.h"
#include "thaf/Application/AppComponent.h"
#include "thaf/Application/Application.h"
#include "thaf/Utils/Debugging/Debug.h"
#include <cassert>

namespace thaf {
using util::IDManager;
using threading::TimerManager;
namespace app {

static AppComponent* activeComponent()
{
    if(!AppComponent::getThreadLocalInstance())
    {
        thafErr("Invoking timer's function on no component");
    }
    return AppComponent::getThreadLocalInstance();
}

Timer::Timer() : _id(TimerManager::invalidJobID()), _cyclic(false)
{
}

Timer::~Timer()
{
    stop();
}

void Timer::start(Timer::Duration milliseconds, TimeOutCallback callback)
{
    if(!callback)
    {
        thafErr("[Timer]: Please specify not null callback");
    }
    else if(activeComponent())
    {
        _myMgr = activeComponent()->getTimeManager();
        if(running())
        {
            thafInfo("Timer is still running, then stop!");
            stop();
        }
        auto component = activeComponent();
        auto onTimeout = [component, callback, this]()
        {
            auto msg = std::make_shared<TimeoutMessage>();
            msg->timerID = _id;
            msg->callback = callback;
            component->postMessage(msg);
            if(!_cyclic)
            {
                _id = TimerManager::invalidJobID(); //mark that timer is not running anymore
            }
        };

        _id = _myMgr->start(milliseconds, onTimeout, _cyclic);
    }
}

void Timer::restart()
{
    if(_myMgr)
    {
        _myMgr->restart(_id);
    }
}

void Timer::stop()
{
    if(_myMgr)
    {
        _myMgr->stop(_id);
    }
}

bool Timer::running()
{
    return _myMgr && _myMgr->isRunning(_id);
}

void Timer::setCyclic(bool cyclic)
{
	if (cyclic != _cyclic)
	{
		_cyclic = cyclic;
        if(_myMgr)
        {
            _myMgr->setCyclic(_id, cyclic);
        }
	}
}


}
}
