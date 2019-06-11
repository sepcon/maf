#include "headers/Framework/Application/Timer.h"
#include "headers/Libs/Threading/Interfaces/BusyTimer.h"
#include "headers/Framework/Application/AppComponent.h"
#include "headers/Framework/Application/Application.h"
#include <iostream>
#include <cassert>

namespace thaf {
using util::IDManager;
namespace app {

static AppComponent* activeComponent()
{
    auto comp = AppComponent::getThreadLocalInstance();
	if (!comp)
	{
		return &Application::instance();
	}
    //assert((comp != nullptr) && "Seems that component of this timer is not a AppComponent or the thread of AppComponent has exited");
    return comp;
}

Timer::Timer() : _id(IDManager::INVALID_ID), _cyclic(false)
{
}

Timer::~Timer()
{
    if(activeComponent())
    {
        stop();
    }
}

void Timer::start(Timer::Duration milliseconds, TimeOutCallback callback)
{
    if(!callback)
    {
        activeComponent()->getLogger()->log("[Timer]: Please specify not null callback", logging::LogLevel::ERROR);
    }
    else if(activeComponent())
    {
        if(isRunning())
        {
            stop();
        }
        auto component = activeComponent();
        auto onTimeout = [component, callback, this]()
        {
            if(!_cyclic)
            {
                _id = IDManager::INVALID_ID; //mark that timer is not running anymore
            }
            auto msg = std::make_shared<TimeoutMessage>();
            msg->timerID = _id;
            msg->callback = callback;
            component->postMessage(msg);
        };

        _id = activeComponent()->getTimer()->start(milliseconds, onTimeout, _cyclic);
    }
}

void Timer::restart()
{
    if(_id != IDManager::INVALID_ID)
    {
        activeComponent()->getTimer()->restart(_id);
    }
    else
    {
        activeComponent()->getLogger()->log("Timer is not running", logging::LogLevel::WARNING);
    }
}

void Timer::stop()
{
    if(IDManager::isValidID(_id))
    {
        activeComponent()->getTimer()->stop(_id);
    }
    else
    {
        activeComponent()->getLogger()->log("Timer is not running", logging::LogLevel::WARNING);
    }
}

bool Timer::isRunning()
{
    return IDManager::isValidID(_id) && activeComponent()->getTimer()->isRunning(_id);
}

void Timer::setCyclic(bool cyclic)
{
	if (cyclic != _cyclic)
	{
		_cyclic = cyclic;
		if (IDManager::isValidID(_id))
		{
			activeComponent()->getTimer()->setCyclic(_id, cyclic);
		}
		{
			activeComponent()->getLogger()->log("Timer is not running", logging::LogLevel::WARNING);
		}
	}
}


}
}
