#ifndef TIMER_H
#define TIMER_H

#include "thaf/Patterns/Patterns.h"
#include <functional>
#include <memory>

namespace thaf {
namespace threading {
class TimerManager;
}
namespace app {

//To wish that the timer will work only in thread of its AppComponent, then don't allow any assignment 
class Timer : public pattern::Unasignable
{
public:
    typedef std::function<void()> TimeOutCallback;
    typedef long long Duration;
    Timer();
    ~Timer();
    void start(Duration milliseconds, TimeOutCallback callback);
    void restart();
    void stop();
    bool running();
    void setCyclic(bool cyclic = true);

private:
    std::shared_ptr<threading::TimerManager> _myMgr;
    unsigned int _id;
	bool _cyclic;
};
}
}
#endif // TIMER_H
