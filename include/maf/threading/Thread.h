#ifndef THREAD_H
#define THREAD_H

#include <maf/patterns/Patterns.h>
#include <maf/utils/cppextension/Invoker.h>
#include <maf/export/MafExport_global.h>
#include <thread>

namespace maf {
namespace threading {

class Thread : public pattern::UnCopyable
{
public:
    using OnSignalCallback = std::function<void(int)>;

    template<typename Callable, typename... Args>
    Thread(
        Callable&& f,
        Args&&... args,
        OnSignalCallback sigHandlerCallback = {})
        :
        _sigHandlerCallback{std::move(sigHandlerCallback)},
        _callable{ std::bind(std::forward<Callable>(f),
                            std::forward<Args>(args)...)
        }
    {
    }

    MAF_EXPORT Thread() = default;
    MAF_EXPORT Thread(Thread&& th);
    MAF_EXPORT Thread& operator=(Thread&& th);
    MAF_EXPORT void join();
    MAF_EXPORT void detach();
    MAF_EXPORT bool joinable();
    MAF_EXPORT virtual ~Thread();
    MAF_EXPORT Thread& start();

protected:
    static void regSignals();
    static void onSystemSignal(int sig);
    void takeFrom(Thread&& th);

    OnSignalCallback        _sigHandlerCallback;
    std::function<void()>   _callable;
    std::thread             _thread;
};
}
}
#endif // THREAD_H
