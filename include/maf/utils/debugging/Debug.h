#ifndef HEADERS_LIBS_UTILS_DEBUGGING_DEBUG_H
#define HEADERS_LIBS_UTILS_DEBUGGING_DEBUG_H

#include <sstream>
#include <functional>

namespace maf {
namespace debugging {

enum class LogLevel : char
{
    LEVEL_INFO,
    LEVEL_WARN,
    LEVEL_ERROR,
    LEVEL_FATAL,
    INDISCARDABLE
};
using LoggingFunctionType = std::function<void(const std::string& msg)>;

void initLogging(LogLevel filteredLevel = LogLevel::INDISCARDABLE,
                 LoggingFunctionType outLogFunc = {},
                 LoggingFunctionType errLogFunc = {});
void log(const std::string& msg, LogLevel filteredLevel);

}
}


#if defined(__clang__) || defined (__GNUC__)
#    define maf_FUNC __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#    define maf_FUNC __FUNCSIG__
#else
#    define maf_FUNC ""
#endif

#ifdef ENABLE_MAF_DEBUG_LOG_VERBOSE
#    define maf_CODE_INFO() "\t\t: " << __FILE__ << ":" << __LINE__ << ":("<< maf_FUNC << ")" << ":"
#else
#    define maf_CODE_INFO() ""
#endif

#define mafInfo(messageChain)  sendToMyLoggingDevice("INFO :    " << messageChain, maf::debugging::LogLevel::LEVEL_INFO, maf_CODE_INFO())
#define mafWarn(messageChain)  sendToMyLoggingDevice("WARN :    " << messageChain, maf::debugging::LogLevel::LEVEL_WARN, maf_CODE_INFO())
#define mafErr(messageChain)   sendToMyLoggingDevice("ERROR:    " << messageChain, maf::debugging::LogLevel::LEVEL_ERROR, maf_CODE_INFO())
#define mafFatal(messageChain) sendToMyLoggingDevice("FATAL:    " << messageChain, maf::debugging::LogLevel::LEVEL_FATAL, maf_CODE_INFO())
#define mafMsg(messageChain)   sendToMyLoggingDevice("MSG  :    " << messageChain, maf::debugging::LogLevel::INDISCARDABLE, "")

#define sendToMyLoggingDevice(messageChain, loglevel, debugInfo)  \
{ \
    std::ostringstream oss; \
    oss << messageChain << debugInfo; \
    maf::debugging::log(oss.str(), loglevel); \
} void(0)

#endif // HEADERS_LIBS_UTILS_DEBUGGING_DEBUG_H
