#ifndef WAITER_H
#define WAITER_H

#include <chrono>
#include <functional>

namespace thaf {
namespace Threading {
class Waiter
{
    class WaiterImpl* _pImpl;
public:
    using CallbackType = std::function<void()>;
    Waiter();
    ~Waiter();
    void waitUtil(std::chrono::system_clock::time_point when, CallbackType callback);
    void waitFor(std::chrono::milliseconds ms, CallbackType callback);
    void stop();
    bool isRunning();
};
}
}


#endif // WAITER_H
