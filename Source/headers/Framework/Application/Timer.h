#ifndef TIMER_H
#define TIMER_H

#include "headers/Framework/Application/Messages.h"
#include "headers/Libs/Patterns/Patterns.h"
#include <functional>
#include <memory>

namespace thaf {
namespace app {

//To wish that the timer will work only in thread of its AppComponent, then don't allow any assignment 
class Timer : pattern::UnCopyable, public pattern::UnMovable 
{
public:
    typedef std::function<void()> TimeOutCallback;
    typedef long long Duration;
    Timer();
    ~Timer();
    void start(Duration milliseconds, TimeOutCallback callback);
    void restart();
    void stop();
    bool isRunning();
    void setCyclic(bool cyclic = true);

private:
    unsigned int _id;
	bool _cyclic;
};
}
}
#endif // TIMER_H
