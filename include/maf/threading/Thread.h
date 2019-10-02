#ifndef THREAD_H
#define THREAD_H

#include <maf/patterns/Patterns.h>
#include <thread>
#include <functional>

namespace maf {
namespace threading {

class Thread : public pattern::UnCopyable
{
protected:
    template<class Callable, class... Args>
    struct Invoker
    {
        std::tuple<Callable, Args...> _callableAndParams;

        Invoker(Callable&& f, Args&&... args) : _callableAndParams {std::forward<Callable>(f), std::forward<Args>(args)...}
        {
        }

        template<class Tuple, size_t... index>
        static void invoke_(Tuple&& t, std::index_sequence<index...>)
        {
            std::invoke(std::move(std::get<index>(t))...);
        }

        void invoke()
        {
            invoke_(std::move(_callableAndParams), std::make_index_sequence<1 + sizeof...(Args)>());
        }
    };
    template<typename Callable, typename... Args>
    static Invoker<std::decay_t<Callable>, std::decay_t<Args>...> makeInvoker(Callable&& f, Args&&... args)
    {
        return Invoker<std::decay_t<Callable>, std::decay_t<Args>...> {std::forward<Callable>(f), std::forward<Args>(args)...};
    }

public:
    using OnSignalCallback = std::function<void(int)>;
    Thread() = default;
    Thread(Thread&& th);
    Thread& operator=(Thread&& th);
    template<typename Callable, typename... Args>
    Thread(Callable&& f, Args&&... args)
    {
        auto invoker = makeInvoker(std::forward<Callable>(f), std::forward<Args>(args)...);
        _callable = [invoker = std::move(invoker), sigHandler = std::move(_sigHandlerCallback)] () mutable {
            _tlSigHandlerCallback = std::move(sigHandler);
            regSignals();
            invoker.invoke();
        };
    }

    Thread &start();
    void join();
    void detach();
    bool joinable();
    void setSignalHandler(OnSignalCallback sigHandlerCallback);
protected:
    static void regSignals();
    static void onSystemSignal(int sig);
    void takeFrom(Thread&& th);

    static thread_local OnSignalCallback _tlSigHandlerCallback;
    std::thread _thread;
    std::function<void()> _callable;
    OnSignalCallback _sigHandlerCallback;
};
}
}
#endif // THREAD_H
