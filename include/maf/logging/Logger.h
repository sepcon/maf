#ifndef HEADERS_LIBS_UTILS_DEBUGGING_DEBUG_H
#define HEADERS_LIBS_UTILS_DEBUGGING_DEBUG_H

#include <sstream>
#include <functional>

namespace maf {
namespace logging {

using LogLevels = uint8_t;

enum LogLevel : uint8_t
{
    LOG_LEVEL_SILENCE      = 0,
    LOG_LEVEL_DEBUG        = 1,
    LOG_LEVEL_INFO         = 2,
    LOG_LEVEL_WARN         = 4,
    LOG_LEVEL_ERROR        = 8,
    LOG_LEVEL_FATAL        = 16,
    LOG_LEVEL_VERBOSE      = 32,
    LOG_LEVEL_FROM_ERROR   = LOG_LEVEL_ERROR | LOG_LEVEL_FATAL,
    LOG_LEVEL_FROM_WARN    = LOG_LEVEL_WARN  | LOG_LEVEL_FROM_ERROR,
    LOG_LEVEL_FROM_INFO    = LOG_LEVEL_INFO  | LOG_LEVEL_FROM_WARN,
};

class Logger
{
public:
    using LoggingFunctionType = std::function<void(const std::string& msg)>;

    static void init(
        LogLevels allowedLevels = LOG_LEVEL_SILENCE,
        LoggingFunctionType outLogFunc = {},
        LoggingFunctionType errLogFunc = {} );

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
    template<typename... Msg>
    static void verbose(Msg&& ... msg);

private:
    template<typename... Msg>
    static void log(LogLevel level, Msg&&... msg);
    static void logImpl(LogLevel filteredLevel, const std::string& msg);
    static bool allowed(LogLevel level);
};

template<typename EnumType,
         std::enable_if_t<std::is_enum_v<EnumType>, bool> = true
         >
inline std::ostream& operator<<(std::ostream& ostr, EnumType code)
{
    return ostr << static_cast<int>(code);
}

template<typename... Msg>
void Logger::log(LogLevel level, Msg&&... msg)
{
    if( allowed(level) )
    {
        std::ostringstream oss;
        (oss << ... << std::forward<Msg>(msg));
        logImpl(level, oss.str());
    }
}

template<typename ...Msg>
void Logger::debug(Msg&&... msg)
{
    log(LOG_LEVEL_DEBUG, "DEBUG :    ", std::forward<Msg>(msg)...);
}
template<typename ...Msg>
void Logger::info(Msg&&... msg)
{
    log(LOG_LEVEL_INFO, "INFO :    ", std::forward<Msg>(msg)...);
}
template<typename ...Msg>
void Logger::warn(Msg&&... msg)
{
    log(LOG_LEVEL_WARN, "WARN :    ", std::forward<Msg>(msg)...);
}
template<typename ...Msg>
void Logger::error(Msg&&... msg)
{
    log(LOG_LEVEL_ERROR, "ERROR :    ", std::forward<Msg>(msg)...);
}
template<typename ...Msg>
void Logger::fatal(Msg&&... msg)
{
    log(LOG_LEVEL_FATAL, "FATAL :    ", std::forward<Msg>(msg)...);
}
template<typename ...Msg>
void Logger::verbose(Msg&&... msg)
{
    log(LOG_LEVEL_FATAL, "VERBOSE:   ", std::forward<Msg>(msg)...);
}

}
}


#endif // HEADERS_LIBS_UTILS_DEBUGGING_DEBUG_H
