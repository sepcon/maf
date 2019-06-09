#ifndef TIMER_H
#define TIMER_H

#include "headers/Application/Framework/Messages.h"
#include <functional>
#include <memory>

namespace thaf {
namespace app {

class Timer
{
public:
    typedef std::function<void()> TimeOutCallback;
    typedef long long Duration;
    Timer();
    ~Timer();
    void start(Duration milliseconds, TimeOutCallback callback, bool cyclic = false);
    void restart();
    void stop();
    bool isRunning();
    void setCyclic(bool cyclic = true);

private:
    unsigned int _id;
};
}
}
#endif // TIMER_H
