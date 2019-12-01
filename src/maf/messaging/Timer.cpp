#include <maf/messaging/Timer.h>
#include <maf/messaging/Component.h>
#include <maf/messaging/BasicMessages.h>
#include <maf/logging/Logger.h>
#include "TimerManager.h"
#include <cassert>

namespace maf {  using logging::Logger;
namespace messaging {

class TimerImpl
{
public:
    typedef std::function<void()> TimeOutCallback;
    typedef long long Duration;
    TimerImpl(bool cyclic = false);
    ~TimerImpl();
    void start(Duration milliseconds, TimeOutCallback callback);
    void restart();
    void stop();
    bool running();
    void setCyclic(bool cyclic = true);

private:
    std::shared_ptr<TimerManager> _myMgr;
    TimerManager::JobID _id;
    bool _cyclic;
};

TimerImpl::TimerImpl(bool cyclic) : _id(TimerManager::invalidJobID()), _cyclic(cyclic)
{
}

TimerImpl::~TimerImpl()
{
    stop();
}

void TimerImpl::start(TimerImpl::Duration milliseconds, TimeOutCallback callback)
{
    if(!callback)
    {
        Logger::error("[TimerImpl]: Please specify not null callback");
    }
    else if( _myMgr || ( !_myMgr && (_myMgr = Component::getTimerManager())))
    {
        if(running())
        {
            Logger::info("TimerImpl is still running, then stop!");
            stop();
        }
        auto componentRef = Component::getActiveWeakPtr();
        auto onTimeout = [componentRef, callback = std::move(callback), this]()
        {
            if(auto component = componentRef.lock())
            {
                component->postMessage<TimeoutMessage>(_id, std::move(callback));
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
        Logger::info("Start new timer with id = " ,  _id);
    }
}

void TimerImpl::restart()
{
    if(_myMgr)
    {
        _myMgr->restart(_id);
    }
}

void TimerImpl::stop()
{
    if(_myMgr)
    {
        _myMgr->stop(_id);
    }
}

bool TimerImpl::running()
{
    return _myMgr && _myMgr->isRunning(_id);
}

void TimerImpl::setCyclic(bool cyclic)
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

Timer::Timer(bool cyclic):
    _pI{std::make_unique<TimerImpl>(cyclic)}
{
}

Timer::~Timer() = default;

void Timer::start(Timer::Duration milliseconds, Timer::TimeOutCallback callback)
{
    _pI->start(milliseconds, std::move(callback));
}

void Timer::restart()
{
    _pI->restart();
}

void Timer::stop()
{
    _pI->stop();
}

bool Timer::running()
{
    return _pI->running();
}

void Timer::setCyclic(bool cyclic)
{
    _pI->setCyclic(cyclic);
}


}
}
