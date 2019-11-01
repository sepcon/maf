#ifndef HEADERS_LIBS_UTILS_DEBUGGING_DEBUG_H
#define HEADERS_LIBS_UTILS_DEBUGGING_DEBUG_H

#include <sstream>
#include <functional>

namespace maf {
namespace logging {

enum class LogLevel : char
{
    LL_INFO,
    LL_WARN,
    LL_ERROR,
    LL_FATAL,
    LL_DEBUG
};

using LoggingFunctionType = std::function<void(const std::string& msg)>;

class Logger
{
public:
    static void initLogging(LogLevel filteredLevel = LogLevel::LL_WARN,
                     LoggingFunctionType outLogFunc = {},
                     LoggingFunctionType errLogFunc = {});

    template<typename... Msg>
    static void debug(Msg&& ... msg);
    template<typename... Msg>
    static void info(Msg&& ... msg);
    template<typename... Msg>
    static void warn(Msg&& ... msg);
    template<typename... Msg>
    static void error(Msg&& ... msg);
    template<typename... Msg>
    static void fatal(Msg&& ... msg);

private:
    template<typename... Msg>
    static void log(LogLevel level, Msg&&... msg);
    static bool allowLog(LogLevel level);
    static void logImpl(LogLevel filteredLevel, const std::string& msg);
};

template<typename... Msg>
void Logger::log(LogLevel level, Msg&&... msg)
{
    if( allowLog(level) )
    {
        std::ostringstream oss;
        (oss << ... << std::forward<Msg>(msg));
        logImpl(level, oss.str());
    }
}

template<typename ...Msg>
void Logger::debug(Msg&&... msg)
{
    log(LogLevel::LL_DEBUG, "DEBUG :    ", std::forward<Msg>(msg)...);
}
template<typename ...Msg>
void Logger::info(Msg&&... msg)
{
    log(LogLevel::LL_INFO, "INFO :    ", std::forward<Msg>(msg)...);
}
template<typename ...Msg>
void Logger::warn(Msg&&... msg)
{
    log(LogLevel::LL_WARN, "WARN :    ", std::forward<Msg>(msg)...);
}
template<typename ...Msg>
void Logger::error(Msg&&... msg)
{
    log(LogLevel::LL_ERROR, "ERROR :    ", std::forward<Msg>(msg)...);
}
template<typename ...Msg>
void Logger::fatal(Msg&&... msg)
{
    log(LogLevel::LL_FATAL, "FATAL :    ", std::forward<Msg>(msg)...);
}

}
}


#endif // HEADERS_LIBS_UTILS_DEBUGGING_DEBUG_H
