#ifndef BUSYTIMER_H
#define BUSYTIMER_H

#include <functional>

namespace Threading
{

class BusyTimer
{
public:
    typedef unsigned long long TimerID;
    typedef long MS;
    typedef std::function<void(TimerID)> TimeOutCallback;
    BusyTimer();
    ~BusyTimer();
    void start(TimerID tid, MS ms, TimeOutCallback callback);
    void stop(TimerID tid);
    bool isRunning(TimerID tid);

private:
    struct BusyTimerImpl* _pImpl;
    BusyTimer(const BusyTimer&) = delete;
    BusyTimer(BusyTimer&&) = delete;
    BusyTimer& operator=(const BusyTimer&) = delete ;
    BusyTimer& operator=(BusyTimer&&) = delete ;
};

}
#endif // BUSYTIMER_H
