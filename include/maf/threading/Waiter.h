#ifndef WAITER_H
#define WAITER_H

#include <maf/export/MafExport_global.h>
#include <chrono>
#include <functional>

namespace maf {
namespace threading {

class Waiter
{
    class WaiterImpl* _pImpl;
public:
    using CallbackType = std::function<void()>;
    MAF_EXPORT Waiter();

    MAF_EXPORT ~Waiter();

    MAF_EXPORT void waitUtil(
        std::chrono::system_clock::time_point when,
        CallbackType callback
        );

    MAF_EXPORT void waitFor(
        std::chrono::milliseconds ms,
        CallbackType callback
        );

    MAF_EXPORT void stop();

    MAF_EXPORT bool isRunning();
};
}
}


#endif // WAITER_H
