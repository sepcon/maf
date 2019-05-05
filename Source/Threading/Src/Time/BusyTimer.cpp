#include "Interfaces/BusyTimer.h"
#include "Prv/Time/BusyTimerImpl.h"
#include <iostream>

namespace Threading
{

BusyTimer::BusyTimer()
{
    _pImpl = new BusyTimerImpl;
}

BusyTimer::~BusyTimer()
{
    if(_pImpl)
    {
        _pImpl->shutdown();
        delete _pImpl;
    }
}

void BusyTimer::start(BusyTimer::TimerID tid, MS ms, std::function<void (BusyTimer::TimerID)> callback)
{
    _pImpl->start(tid, ms, callback);
}

void BusyTimer::stop(BusyTimer::TimerID tid)
{
    std::cout << "Trying to stop this timer " << tid << std::endl;
    _pImpl->stop(tid);
}

bool BusyTimer::isRunning(BusyTimer::TimerID tid)
{
    return _pImpl->isRunning(tid);
}

}
