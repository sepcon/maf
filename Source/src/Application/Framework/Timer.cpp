#include "headers/Application/Framework/Timer.h"
#include "headers/Threading/Interfaces/BusyTimer.h"
#include "headers/Application/Framework/AppComponent.h"
#include <iostream>
#include <cassert>

#define JOBID_FROM_PTR(pointer) (reinterpret_cast<BusyTimer::JobID>(pointer))

namespace thaf {
using Threading::BusyTimer;
namespace app {

static AppComponent* activeComponent()
{
    auto comp = Component::getThreadLocalInstance<AppComponent>();
    assert((comp != nullptr) && "Seems that component of this timer is not an AppComponent");
    return comp;
}

Timer::Timer() : _id(IDManager::INVALID_ID)
{
}

Timer::~Timer()
{
}

void Timer::start(Timer::Duration milliseconds, TimeOutCallback callback, bool cyclic)
{
    if(!callback)
    {
        //err message;
    }
    else if(activeComponent())
    {
        auto packagedComponent = activeComponent();
        auto onTimeout = [packagedComponent, callback]()
        {
            auto msg = std::make_shared<TimeoutMessage>();
            msg->callback = callback;
            packagedComponent->postMessage(msg);
        };

        _id = activeComponent()->getTimer()->start(milliseconds, onTimeout, cyclic);
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
        //err message here
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
    //err message here
    }
}

bool Timer::isRunning()
{
    return IDManager::isValidID(_id) && activeComponent()->getTimer()->isRunning(_id);
}

void Timer::setCyclic(bool cyclic)
{
    if(IDManager::isValidID(_id))
    {
        Component::getThreadLocalInstance<AppComponent>()->getTimer()->setCyclic(_id, cyclic);
    }
}


}
}
