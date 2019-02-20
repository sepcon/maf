#ifndef TIMER_H
#define TIMER_H


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
    void setInterval(int milliseconds);
    void start();
};
}

#endif // TIMER_H
