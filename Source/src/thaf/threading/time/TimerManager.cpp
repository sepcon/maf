#include "thaf/threading/TimerManager.h"
#include "thaf/threading/internal/TimerManagerImpl.h"
#include "thaf/utils/debugging/Debug.h"

namespace thaf {
namespace threading {

TimerManager::TimerManager()
{
    _pImpl = new TimerManagerImpl;
}

TimerManager::~TimerManager()
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
            thafWarn( "Caught exception: " << e.what() );
        }
        catch (...)
        {
            thafWarn( "Uncaught exception!" );
        }
    }
}

void TimerManager::restart(JobID jid)
{
    if(isValid(jid))
    {
        _pImpl->restart(jid);
    }
}

TimerManager::JobID TimerManager::start(Duration milliseconds, TimeOutCallback callback, bool cyclic)
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

void TimerManager::stop(TimerManager::JobID jid)
{
    if(isValid(jid))
    {
        _idManager.reclaimUsedID(jid);
        _pImpl->stop(jid);
    }
}

bool TimerManager::isRunning(TimerManager::JobID jid)
{
    return isValid(jid) && _pImpl->isRunning(jid);
}

void TimerManager::setCyclic(TimerManager::JobID jid, bool cyclic)
{
    if(isValid(jid))
    {
        _pImpl->setCyclic(jid, cyclic);
    }
}

bool TimerManager::isValid(TimerManager::JobID jid)
{
    return util::IDManager::isValidID(jid);
}

}
}
