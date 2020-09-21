#pragma once

#include <maf/export/MafExport_global.h>
#include <maf/patterns/Patterns.h>
#include <maf/utils/ExecutorIF.h>

#include <chrono>

namespace maf {
namespace messaging {

class SyncTimer : public pattern::Unasignable {
  using ExecutorIFPtr = std::shared_ptr<util::ExecutorIF>;

 public:
  typedef std::function<void()> TimeOutCallback;
  MAF_EXPORT SyncTimer(bool cyclic = false);
  MAF_EXPORT ~SyncTimer();
  MAF_EXPORT void start(long long milliseconds, TimeOutCallback callback,
                        ExecutorIFPtr = {});
  MAF_EXPORT void start(std::chrono::milliseconds milliseconds,
                        TimeOutCallback callback, ExecutorIFPtr = {});

  MAF_EXPORT void restart();
  MAF_EXPORT void stop();
  MAF_EXPORT bool running();
  MAF_EXPORT void setCyclic(bool cyclic = true);

 private:
  struct SyncTimerDataPrv *d_;
};

}  // namespace messaging
}  // namespace maf
