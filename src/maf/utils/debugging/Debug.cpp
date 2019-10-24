#include <maf/utils/debugging/Debug.h>


namespace maf {
namespace debugging {

namespace
{
struct Statics
{
    LoggingFunctionType out = [](const std::string&) {};
    LoggingFunctionType err = [](const std::string&) {};
    LogLevel filteredLevel = LogLevel::INDISCARDABLE;
};
static Statics& statics()
{
    static Statics s;
    return s;
};
}

void initLogging(LogLevel filteredLevel, LoggingFunctionType outLogFunc, LoggingFunctionType errLogFunc)
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

void log(const std::string &msg, LogLevel filteredLevel)
{
    if(statics().filteredLevel <= filteredLevel)
    {
        switch (filteredLevel)
        {
        case LogLevel::LEVEL_INFO:
        case LogLevel::INDISCARDABLE:
            statics().out(msg);
            break;
        default:
            statics().err(msg);
            break;
        }
    }
}

}
}
