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

void BusyTimer::restart(JobID jid)
{
    _pImpl->restart(jid);
}

void BusyTimer::start(BusyTimer::JobID jid, Duration milliseconds, std::function<void (BusyTimer::JobID)> callback, bool cyclic)
{
    _pImpl->start(jid, milliseconds, callback, cyclic);
}

void BusyTimer::stop(BusyTimer::JobID jid)
{
    std::cout << "Trying to stop this timer " << jid << std::endl;
    _pImpl->stop(jid);
}

bool BusyTimer::isRunning(BusyTimer::JobID jid)
{
    return _pImpl->isRunning(jid);
}

void BusyTimer::setCyclic(BusyTimer::JobID jid, bool cyclic)
{
    _pImpl->setRecyclic(jid, cyclic);
}

}
