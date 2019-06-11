#ifndef THREAD_H
#define THREAD_H

#include <thread>
#include <signal.h>

namespace thaf {
namespace threading {

class Thread
{
public:
    Thread() = default;
    Thread(Thread&& th);
    virtual ~Thread(){}
    template<typename Callable, typename... Args>
    Thread(Callable&& f, Args&&... args)
    {
        auto call = [&f, &args..., this]{
            Thread::_this = this;
            regSignals();
            f(std::forward<Args>(args)...);
        };
        _thread = std::thread(call);
    }

    void join();
    void detach();
    bool joinable();
private:
    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;
    Thread& operator=(Thread&& th) = delete;

    static void regSignals()
    {
        signal(SIGABRT, &Thread::onSystemSignal);
        signal(SIGSEGV, &Thread::onSystemSignal);
        signal(SIGILL,  &Thread::onSystemSignal);
        signal(SIGTERM, &Thread::onSystemSignal);
    }
    static void onSystemSignal(int sig)
    {
        if(_this)
        {
            _this->handleSignal(sig);
        }
    }
    virtual void handleSignal(int sig);

    static thread_local Thread* _this;
    std::thread _thread;
};
};
}
#endif // THREAD_H
