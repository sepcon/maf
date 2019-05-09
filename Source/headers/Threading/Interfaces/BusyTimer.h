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
    void start(JobID tid, Duration milliseconds, TimeOutCallback callback);
    void restart(JobID tid);
    void stop(JobID tid);
    bool isRunning(JobID tid);

private:
    struct BusyTimerImpl* _pImpl;
    BusyTimer(const BusyTimer&) = delete;
    BusyTimer(BusyTimer&&) = delete;
    BusyTimer& operator=(const BusyTimer&) = delete ;
    BusyTimer& operator=(BusyTimer&&) = delete ;
};

}
#endif // BUSYTIMER_H
