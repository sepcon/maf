#pragma once

#include <maf/export/MafExport_global.h>
#include <maf/patterns/Patterns.h>
#include <maf/utils/ExecutorIF.h>

#include <chrono>
#include <functional>
#include <memory>

namespace maf {
namespace messaging {

class Timer : public pattern::Unasignable {
  using ExecutorIFPtr = std::shared_ptr<util::ExecutorIF>;

 public:
  typedef std::function<void()> TimeOutCallback;
  MAF_EXPORT Timer(bool cyclic = false);
  MAF_EXPORT ~Timer();
  MAF_EXPORT void start(long long milliseconds, TimeOutCallback callback,
                        ExecutorIFPtr = {});
  MAF_EXPORT void start(std::chrono::milliseconds milliseconds,
                        TimeOutCallback callback, ExecutorIFPtr = {});

  MAF_EXPORT void restart();
  MAF_EXPORT void stop();
  MAF_EXPORT bool running() const;
  MAF_EXPORT void setCyclic(bool cyclic = true);
  MAF_EXPORT static void timeoutAfter(long long milliseconds,
                                      TimeOutCallback callback,
                                      ExecutorIFPtr executor = {});
  MAF_EXPORT static void timeoutAfter(std::chrono::milliseconds milliseconds,
                                      TimeOutCallback callback,
                                      ExecutorIFPtr executor = {});

 private:
  std::shared_ptr<struct TimerData> d_;
};

}  // namespace messaging
}  // namespace maf
