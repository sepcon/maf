#pragma once

#include <chrono>
#include <functional>

namespace maf {
namespace util {

struct TimeCounter {
  using SystemClock = std::chrono::system_clock;
  using TimePoint = SystemClock::time_point;
  using MicroSeconds = std::chrono::microseconds;

  void restart() { _startTime = std::chrono::system_clock::now(); }

  MicroSeconds elapsedTime() const {
    return std::chrono::duration_cast<MicroSeconds>(
        std::chrono::system_clock::now() - _startTime);
  }
  template <class _Duration>
  _Duration elapsedTime() const {
    return std::chrono::duration_cast<_Duration>(
        std::chrono::system_clock::now() - _startTime);
  }

 protected:
  TimePoint _startTime = SystemClock::now();
};

class TimeMeasurement : public TimeCounter {
 public:
  using MicroSeconds = std::chrono::microseconds;
  TimeMeasurement() = default;
  TimeMeasurement(std::function<void(MicroSeconds)> onReportCallback)
      : _onReportCallback(std::move(onReportCallback)) {}
  TimeMeasurement(TimeMeasurement&&) = default;
  TimeMeasurement& operator=(TimeMeasurement&&) = default;
  TimeMeasurement(const TimeMeasurement&) = delete;
  TimeMeasurement& operator=(const TimeMeasurement&) = delete;

  ~TimeMeasurement() { stop(); }

  MicroSeconds stop() {
    auto elapsed = this->elapsedTime();
    if (_onReportCallback) {
      _onReportCallback(elapsed);
      _onReportCallback = nullptr;
    }
    return elapsed;
  }

  MicroSeconds abort() {
    _onReportCallback = {};
    return elapsedTime();
  }

 protected:
  std::function<void(MicroSeconds)> _onReportCallback;
};

}  // namespace util
}  // namespace maf
