#ifndef BUSYTIMER_H
#define BUSYTIMER_H

#include "headers/Utils/IDManager.h"
#include <functional>

namespace thaf {
namespace Threading {

class BusyTimer
{
public:
    using JobID = thaf::IDManager::IDType;
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
    thaf::IDManager _idManager;
    BusyTimer(const BusyTimer&) = delete;
    BusyTimer(BusyTimer&&) = delete;
    BusyTimer& operator=(const BusyTimer&) = delete ;
    BusyTimer& operator=(BusyTimer&&) = delete ;
};

}
}
#endif // BUSYTIMER_H
