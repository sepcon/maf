#ifndef TIMER_H
#define TIMER_H

/***
 * To wish that the timer will work only in thread of its Component, then don't allow any assignment
*/

#include <maf/patterns/Patterns.h>
#include <functional>
#include <memory>

namespace maf {
namespace messaging {

class TimerManager;
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
    std::shared_ptr<messaging::TimerManager> _myMgr;
    unsigned int _id;
    bool _cyclic;
};
}
}
#endif // TIMER_H
