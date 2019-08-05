#include "maf/logging/LoggingComponent.h"
#include "maf/logging/LoggerBase.h"
#include "maf/logging/ConsoleLogger.h"

namespace maf {
namespace logging {


LoggingComponent::LoggingComponent(Invisible)
{
    start();
}

void LoggingComponent::init()
{
    std::lock_guard<std::mutex> lock(m);
    _loggers[Console] = new ConsoleLogger;
}

ILogger* LoggingComponent::getLogger(LoggerDestinationName dest)
{
    std::lock_guard<std::mutex> lock(m);
    auto itLogger = _loggers.find(dest);
    if(itLogger == _loggers.end())
    {
        return &NullLogger::instance();
    }
    else
    {
        return itLogger->second;
    }
}

void LoggingComponent::registerLogger(LoggerDestinationName dest, ILogger *pLogger)
{
    std::lock_guard<std::mutex> lock(m);
    _loggers[dest] = pLogger;
}

} // logging
} // maf
