#pragma once

#include <maf/utils/SignalSlots.h>

#include "Timer.h"

namespace maf {
namespace messaging {

class SignalTimer {
 public:
  using Milliseconds = std::chrono::milliseconds;
  using TimeoutSignal = signal_slots::Signal<>;

  void start(long long interval) { start(Milliseconds{interval}); }

  void start(Milliseconds interval) {
    impl_.start(interval, [this] { timeoutSignal(); });
  }

  void restart() { impl_.restart(); }

  void stop() { impl_.stop(); }

  bool running() const { return impl_.running(); }

  void setCyclic(bool yes = true) { impl_.setCyclic(yes); }

  TimeoutSignal timeoutSignal;

 private:
  Timer impl_;
};

}  // namespace messaging
}  // namespace maf
