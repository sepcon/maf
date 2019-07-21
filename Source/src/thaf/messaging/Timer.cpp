#include "thaf/messaging/Timer.h"
#include "thaf/threading/TimerManager.h"
#include "thaf/messaging/Component.h"
#include "thaf/messaging/BasicMessages.h"
#include "thaf/utils/debugging/Debug.h"
#include <cassert>

namespace thaf {
using util::IDManager;
using threading::TimerManager;
namespace messaging {


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
    else if((_myMgr = Component::getTimeManager()))
    {
        if(running())
        {
            thafInfo("Timer is still running, then stop!");
            stop();
        }
        auto componentRef = Component::getComponentRef();
        auto onTimeout = [componentRef, callback, this]()
        {
            auto msg = std::make_shared<TimeoutMessage>();
            msg->timerID = _id;
            msg->callback = callback;
            auto lock(componentRef->pa_lock());
            if(componentRef->get())
            {
                componentRef->get()->postMessage(msg);
            }
            else
            {
                if(_cyclic && _myMgr)
                {
                    _myMgr->stop(_id);
                }
                _id = TimerManager::invalidJobID();
            }
            if(!_cyclic)
            {
                _id = TimerManager::invalidJobID(); //mark that timer is not running anymore
            }
        };

        _id = _myMgr->start(milliseconds, onTimeout, _cyclic);
        thafInfo("Start new timer with id = " << _id);
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
