#pragma once

#include "ExecutorIF.h"
#include <chrono>
#include <maf/export/MafExport_global.h>
#include <maf/patterns/Patterns.h>

namespace maf {
namespace messaging {

class SyncTimer : public pattern::Unasignable {
  using ExecutorPtr = std::shared_ptr<ExecutorIF>;

public:
  typedef std::function<void()> TimeOutCallback;
  MAF_EXPORT SyncTimer(bool cyclic = false);
  MAF_EXPORT ~SyncTimer();
  MAF_EXPORT void start(long long milliseconds, TimeOutCallback callback,
                        ExecutorPtr = {});
  MAF_EXPORT void start(std::chrono::milliseconds milliseconds,
                        TimeOutCallback callback, ExecutorPtr = {});

  MAF_EXPORT void restart();
  MAF_EXPORT void stop();
  MAF_EXPORT bool running();
  MAF_EXPORT void setCyclic(bool cyclic = true);

private:
  struct SyncTimerDataPrv *d_;
};

} // namespace messaging
} // namespace maf
