#pragma once

#include "thaf/utils/debugging/Debug.h"
#include <functional>
#include <chrono>

namespace thaf {
namespace util {

class TimeMeasurement
{
public:
    TimeMeasurement(std::function<void(long long)>&& onReportCallback = {}) :
        _onReportCallback(std::move(onReportCallback))
    {
        _startTime = std::chrono::system_clock::now();
    }
    ~TimeMeasurement()
    {
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - _startTime).count();
        thafInfo("Time measurment stopped, elapsed time = " << elapsedTime);
        if(_onReportCallback)
        {
            _onReportCallback(elapsedTime);
        }
    }

    std::function<void(long long)> _onReportCallback;
    std::chrono::system_clock::time_point _startTime;
};
}
}
