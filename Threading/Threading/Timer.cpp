#include "Timer.h"
#include "ThreadJoiner.h"
#include <condition_variable>
#include <thread>
#include <mutex>

namespace Threading
{

class TimerImpl
{
std::mutex _mutex;
std::condition_variable _cond;
std::thread _th;
void threadFunc()
{

}
public:


};

Timer::Timer(int interval)
{

}

void Timer::start(int milliseconds, ITimerExpiredCallBack *pCallback)
{

}

void Timer::start(int milliseconds, std::function<void ()> onExpired)
{

}

void Timer::restart(int milliseconds)
{

}

void Timer::stop()
{

}

}
