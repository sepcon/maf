#ifndef TIMER_H
#define TIMER_H

#include <functional>

namespace Threading
{

class ITimerExpiredCallBack
{
public:
    virtual void onExpired(const class Timer& timer) = 0;
    virtual ~ITimerExpiredCallBack() {}
};

class Timer
{
public:
    Timer(int interval);
    void start(int milliseconds, ITimerExpiredCallBack* pCallback);
    void start(int milliseconds, std::function<void()> onExpired);
    void restart(int milliseconds);
    void stop();
};
}

#endif // TIMER_H
