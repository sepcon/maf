#pragma once
#include "maf/utils/IDManager.h"
#include <functional>

namespace maf {
namespace threading {

class TimerManager
{
public:
    using JobID = util::IDManager::IDType;
    typedef long long Duration;
    typedef std::function<void()> TimeOutCallback;
    TimerManager();
    ~TimerManager();
    JobID start(Duration milliseconds, TimeOutCallback callback, bool cyclic = false);
    void restart(JobID jid);
    void stop(JobID jid);
    bool isRunning(JobID jid);
    void setCyclic(JobID jid, bool cyclic = true);

    static bool isValid(JobID jid);
    static JobID invalidJobID() { return util::IDManager::INVALID_ID; }

private:
    struct TimerManagerImpl* _pImpl;
    util::IDManager _idManager;
    TimerManager(const TimerManager&) = delete;
    TimerManager(TimerManager&&) = delete;
    TimerManager& operator=(const TimerManager&) = delete ;
    TimerManager& operator=(TimerManager&&) = delete ;
};

}
}
