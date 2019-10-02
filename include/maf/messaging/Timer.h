#ifndef TIMER_H
#define TIMER_H

#include <maf/patterns/Patterns.h>
#include <functional>
#include <memory>

namespace maf {
namespace threading { class TimerManager; }
namespace messaging {

//To wish that the timer will work only in thread of its Component, then don't allow any assignment
class Timer : public pattern::Unasignable
{
public:
    typedef std::function<void()> TimeOutCallback;
    typedef long long Duration;
    Timer(bool cyclic = false);
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
