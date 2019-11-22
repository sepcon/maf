#include <maf/logging/Logger.h>


namespace maf {
namespace logging {

namespace
{
struct Statics
{
    Logger::LoggingFunctionType out = [](const std::string&) {};
    Logger::LoggingFunctionType err = [](const std::string&) {};
    LogLevels allowedLevels = LOG_LEVEL_SILENCE;
};

static Statics& statics()
{
    static Statics s;
    return s;
};
}

void Logger::init(LogLevels allowedLevels, LoggingFunctionType outLogFunc, LoggingFunctionType errLogFunc)
{
    if(outLogFunc)
    {
        statics().out = std::move(outLogFunc);
    }
    if(errLogFunc)
    {
        statics().err = std::move(errLogFunc);
    }
    else if(statics().out)
    {
        statics().err = statics().out;
    }
    statics().allowedLevels = allowedLevels;
}

void Logger::logImpl(LogLevel filteredLevel, const std::string &msg)
{
    switch (filteredLevel)
    {
    case LOG_LEVEL_INFO:
    case LOG_LEVEL_DEBUG:
        statics().out(msg);
        break;
    default:
        statics().err(msg);
        break;
    }
}

bool Logger::allowed(LogLevel level)
{
    return statics().allowedLevels & level;
}

}
}
