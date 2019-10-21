#pragma once

#include <functional>
#include <chrono>

namespace maf {
namespace util {

class TimeMeasurement
{
public:
	TimeMeasurement() = default;
    TimeMeasurement(std::function<void(long long)> onReportCallback) :
        _onReportCallback(std::move(onReportCallback))
    {
        _startTime = std::chrono::system_clock::now();
    }
    ~TimeMeasurement()
    {
        stop();
    }

	long long elapsedTime() const 
	{
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - _startTime).count();
	}

    void stop()
    {
        if(_onReportCallback)
        {
            _onReportCallback(this->elapsedTime());
            _onReportCallback = nullptr;
        }
    }
    std::function<void(long long)> _onReportCallback;
    std::chrono::system_clock::time_point _startTime;
};
}
}
