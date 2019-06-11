#ifndef BUSYTIMER_H
#define BUSYTIMER_H

#include "headers/Libs/Utils/IDManager.h"
#include <functional>

namespace thaf {
namespace threading {

class BusyTimer
{
public:
    using JobID = util::IDManager::IDType;
    typedef long long Duration;
    typedef std::function<void()> TimeOutCallback;
    BusyTimer();
    ~BusyTimer();
    JobID start(Duration milliseconds, TimeOutCallback callback, bool cyclic = false);
    void restart(JobID jid);
    void stop(JobID jid);
    bool isRunning(JobID jid);
    void setCyclic(JobID jid, bool cyclic = true);

private:
    struct BusyTimerImpl* _pImpl;
    util::IDManager _idManager;
    BusyTimer(const BusyTimer&) = delete;
    BusyTimer(BusyTimer&&) = delete;
    BusyTimer& operator=(const BusyTimer&) = delete ;
    BusyTimer& operator=(BusyTimer&&) = delete ;
};

}
}
#endif // BUSYTIMER_H
