#ifndef TIMER_H
#define TIMER_H

#include "CallbackExecutorIF.h"
#include <chrono>
#include <maf/export/MafExport_global.h>
#include <maf/patterns/Patterns.h>
#include <memory>

namespace maf {
namespace messaging {

class Timer : public pattern::Unasignable {
  using ExecutorPtr = std::shared_ptr<CallbackExecutorIF>;

public:
  typedef std::function<void()> TimeOutCallback;
  MAF_EXPORT Timer(bool cyclic = false);
  MAF_EXPORT ~Timer();
  MAF_EXPORT void start(long long milliseconds, TimeOutCallback callback,
                        ExecutorPtr = {});
  MAF_EXPORT void start(std::chrono::milliseconds milliseconds,
                        TimeOutCallback callback, ExecutorPtr = {});

  MAF_EXPORT void restart();
  MAF_EXPORT void stop();
  MAF_EXPORT bool running();
  MAF_EXPORT void setCyclic(bool cyclic = true);

private:
  std::unique_ptr<struct TimerDataPrv> d_;
};

} // namespace messaging
} // namespace maf
#endif // TIMER_H
