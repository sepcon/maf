#ifndef THREAD_H
#define THREAD_H

#include <thread>
#include <signal.h>

namespace thaf {
namespace threading {

class Thread final
{
public:
    using OnSignalCallback = std::function<void(int)>;
    Thread() = default;
    Thread(Thread&& th);
    virtual ~Thread(){}
    template<typename Callable, typename... Args>
    Thread(Callable&& f, Args&&... args)
    {
        _callable = [&f, &args..., this]{
            Thread::_this = this;
            regSignals();
            f(std::forward<Args>(args)...);
        };
    }

    Thread &start();
    void join();
    void detach();
    bool joinable();
    void setSignalHandler(OnSignalCallback sigHandlerCallback);
private:
    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;
    Thread& operator=(Thread&& th) = delete;
    static void regSignals();
    static void onSystemSignal(int sig);

    static thread_local Thread* _this;
    std::thread _thread;
	std::function<void()> _callable;
    std::function<void(int)> _sigHandlerCallback;
};
};
}
#endif // THREAD_H
