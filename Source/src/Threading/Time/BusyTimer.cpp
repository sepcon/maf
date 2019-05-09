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
		try
		{
			_pImpl->shutdown();
			delete _pImpl;
		}
		catch (const std::exception& e)
		{
			std::cout << "Caught exception: " << e.what() << std::endl;
		}
		catch (...)
		{
			std::cout << "Uncaught exception!" << std::endl;
		}
    }
}

void BusyTimer::restart(JobID tid)
{
    _pImpl->restart(tid);
}

void BusyTimer::start(BusyTimer::JobID tid, Duration milliseconds, std::function<void (BusyTimer::JobID)> callback)
{
    _pImpl->start(tid, milliseconds, callback);
}

void BusyTimer::stop(BusyTimer::JobID tid)
{
    std::cout << "Trying to stop this timer " << tid << std::endl;
    _pImpl->stop(tid);
}

bool BusyTimer::isRunning(BusyTimer::JobID tid)
{
    return _pImpl->isRunning(tid);
}

}
