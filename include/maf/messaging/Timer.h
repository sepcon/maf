#ifndef TIMER_H
#define TIMER_H

/***
 * To wish that the timer will work only in thread of its Component, then don't allow any assignment
*/

#include <maf/export/MafExport_global.h>
#include <maf/patterns/Patterns.h>
#include <functional>
#include <memory>

namespace maf {
namespace messaging {

class Timer : public pattern::Unasignable
{
public:
    typedef std::function<void()> TimeOutCallback;
    typedef long long Duration;
    MAF_EXPORT Timer(bool cyclic = false);
    MAF_EXPORT ~Timer();
    MAF_EXPORT void start(Duration milliseconds, TimeOutCallback callback);
    MAF_EXPORT void restart();
    MAF_EXPORT void stop();
    MAF_EXPORT bool running();
    MAF_EXPORT void setCyclic(bool cyclic = true);

private:
    std::unique_ptr<struct TimerDataPrv> d_;
};
}
}
#endif // TIMER_H
