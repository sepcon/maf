#include <maf/messaging/Timer.h>
#include <maf/messaging/Component.h>
#include <maf/messaging/BasicMessages.h>
#include <maf/logging/Logger.h>
#include "TimerManager.h"
#include <cassert>

namespace maf {  using logging::Logger;
namespace messaging {

using JobID = TimerManager::JobID;

struct TimerDataPrv
{
    TimerDataPrv(JobID _id, bool _cyclic)
        : id{_id}, cyclic{_cyclic}
    {}

    JobID                   id;
    bool                    cyclic;
};

static TimerManager& ourMgr()
{
    static TimerManager mgr;
    return mgr;
}

Timer::Timer(bool cyclic) :
    d_{new TimerDataPrv{TimerManager::invalidJobID(), cyclic}}
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
        Logger::error("[TimerImpl]: Please specify not null callback");
    }
    else
    {
        if(running())
        {
            Logger::info("TimerImpl is still running, then stop!");
            stop();
        }
        auto componentRef = RunningComponent::weak();
        auto onTimeout = [componentRef, callback = std::move(callback), this]()
        {
            if(auto component = componentRef.lock())
            {
                component->post<CallbackExcMsg>(std::move(callback));
            }
            else
            {
                if(d_->cyclic)
                {
                    ourMgr().stop(d_->id);
                }
                d_->id = TimerManager::invalidJobID();
            }

            if(!d_->cyclic)
            {
                d_->id = TimerManager::invalidJobID(); //mark that timer is not running anymore
            }
        };

        d_->id = ourMgr().start(milliseconds, onTimeout, d_->cyclic);
        Logger::info("Start new timer with id = " ,  d_->id);
    }
}

void Timer::restart()
{
    ourMgr().restart(d_->id);
}

void Timer::stop()
{
    ourMgr().stop(d_->id);
}

bool Timer::running()
{
    return ourMgr().isRunning(d_->id);
}

void Timer::setCyclic(bool cyclic)
{
    if (cyclic != d_->cyclic)
    {
        d_->cyclic = cyclic;
        ourMgr().setCyclic(d_->id, cyclic);
    }
}

}
}
