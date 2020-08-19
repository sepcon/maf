#pragma once

#include <maf/export/MafExport_global.h>
#include <maf/patterns/Patterns.h>

#include <chrono>
#include <functional>
#include <memory>

#include "ExecutorIF.h"

namespace maf {
namespace messaging {

class Timer : public pattern::Unasignable {
  using ExecutorPtr = std::shared_ptr<ExecutorIF>;

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
  std::shared_ptr<struct TimerData> d_;
};

}  // namespace messaging
}  // namespace maf
