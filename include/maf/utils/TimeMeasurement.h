#pragma once

#include <functional>
#include <chrono>

namespace maf {
namespace util {

class TimeMeasurement
{
public:
    using MicroSeconds = std::chrono::microseconds;
    TimeMeasurement() = default;
    TimeMeasurement(std::function<void(MicroSeconds)> onReportCallback) :
        _onReportCallback(std::move(onReportCallback))
    {
        _startTime = std::chrono::system_clock::now();
    }
    ~TimeMeasurement()
    {
        stop();
    }

    MicroSeconds elapsedTime() const
    {
        return std::chrono::duration_cast<MicroSeconds>(std::chrono::system_clock::now() - _startTime);
    }

    MicroSeconds stop()
    {
        auto elapsed = this->elapsedTime();
        if(_onReportCallback)
        {
            _onReportCallback(elapsed);
            _onReportCallback = nullptr;
        }
        return elapsed;
    }

    MicroSeconds abort()
    {
        _onReportCallback = {};
        return elapsedTime();
    }
    std::function<void(MicroSeconds)> _onReportCallback;
    std::chrono::system_clock::time_point _startTime;
};
}
}
