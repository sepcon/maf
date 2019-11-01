#include <maf/logging/Logger.h>


namespace maf {
namespace logging {

namespace
{
struct Statics
{
    LoggingFunctionType out = [](const std::string&) {};
    LoggingFunctionType err = [](const std::string&) {};
    LogLevel filteredLevel = LogLevel::LL_WARN;
};

static Statics& statics()
{
    static Statics s;
    return s;
};
}

void Logger::initLogging(LogLevel filteredLevel, LoggingFunctionType outLogFunc, LoggingFunctionType errLogFunc)
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
    statics().filteredLevel = filteredLevel;
}

void Logger::logImpl(LogLevel filteredLevel, const std::string &msg)
{
    switch (filteredLevel)
    {
    case LogLevel::LL_INFO:
    case LogLevel::LL_DEBUG:
        statics().out(msg);
        break;
    default:
        statics().err(msg);
        break;
    }
}

bool Logger::allowLog(LogLevel level)
{
    return statics().filteredLevel <= level;
}

}
}
