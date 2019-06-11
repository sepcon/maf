#include "headers/Libs/Threading/Interfaces/BusyTimer.h"
#include "headers/Libs/Threading/Prv/Time/BusyTimerImpl.h"
#include <iostream>

namespace thaf {
namespace threading {

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

BusyTimer::JobID BusyTimer::start(Duration milliseconds, TimeOutCallback callback, bool cyclic)
{
    auto jid = _idManager.allocateNewID();
    if(jid != util::IDManager::INVALID_ID)
    {
        auto jobDoneCallback = [callback, this](JobID jid, bool isCyclic)
        {
            if(!isCyclic)
            {
                _idManager.reclaimUsedID(jid);
            }
            if(callback) { callback(); }
        };

        if(!_pImpl->start(jid, milliseconds, jobDoneCallback, cyclic))
        {
            _idManager.reclaimUsedID(jid);
        }
    }
    return jid;
}

void BusyTimer::stop(BusyTimer::JobID jid)
{
    _idManager.reclaimUsedID(jid);
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
}
