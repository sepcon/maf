#ifndef BUSYTIMER_H
#define BUSYTIMER_H

#include <functional>

namespace Threading
{

class BusyTimer
{
public:
    typedef unsigned long long JobID;
    typedef long long Duration;
    typedef std::function<void(JobID)> TimeOutCallback;
    BusyTimer();
    ~BusyTimer();
    void start(JobID jid, Duration milliseconds, TimeOutCallback callback, bool cyclic = false);
    void restart(JobID jid);
    void stop(JobID jid);
    bool isRunning(JobID jid);
    void setCyclic(JobID jid, bool cyclic = true);

private:
    struct BusyTimerImpl* _pImpl;
    BusyTimer(const BusyTimer&) = delete;
    BusyTimer(BusyTimer&&) = delete;
    BusyTimer& operator=(const BusyTimer&) = delete ;
    BusyTimer& operator=(BusyTimer&&) = delete ;
};

}
#endif // BUSYTIMER_H
