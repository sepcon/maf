#pragma once

#include <maf/export/MafExport_global.h>

#include <functional>
#include <sstream>

namespace maf {
namespace logging {

using LogLevels = uint8_t;
using LogLevel = uint8_t;

inline constexpr LogLevel LOG_LEVEL_SILENCE = 0;
inline constexpr LogLevel LOG_LEVEL_INFO = 1;
inline constexpr LogLevel LOG_LEVEL_WARN = 2;
inline constexpr LogLevel LOG_LEVEL_ERROR = 4;
inline constexpr LogLevel LOG_LEVEL_FATAL = 8;
inline constexpr LogLevel LOG_LEVEL_DEBUG = 16;
inline constexpr LogLevel LOG_LEVEL_VERBOSE = 32;
inline constexpr LogLevel LOG_LEVEL_FROM_ERROR =
    LOG_LEVEL_ERROR | LOG_LEVEL_FATAL;
inline constexpr LogLevel LOG_LEVEL_FROM_WARN =
    LOG_LEVEL_WARN | LOG_LEVEL_FROM_ERROR;
inline constexpr LogLevel LOG_LEVEL_FROM_INFO =
    LOG_LEVEL_INFO | LOG_LEVEL_FROM_WARN;

using LoggingFunctionType = std::function<void(const std::string &msg)>;

MAF_EXPORT void init(LogLevels allowedLevels = LOG_LEVEL_SILENCE,
                     LoggingFunctionType outLogFunc = {},
                     LoggingFunctionType errLogFunc = {});

MAF_EXPORT void stopLogging();
MAF_EXPORT void changeLogLevels(LogLevels allowedLevels = LOG_LEVEL_SILENCE);
MAF_EXPORT bool allowed(LogLevel level);
MAF_EXPORT void enable(LogLevel level);
MAF_EXPORT void disable(LogLevel level);
MAF_EXPORT void logImpl(LogLevel filteredLevel, const std::string &msg);

template <typename... Msg>
void debug(Msg &&... msg);
template <typename... Msg>
void info(Msg &&... msg);
template <typename... Msg>
void warn(Msg &&... msg);
template <typename... Msg>
void error(Msg &&... msg);
template <typename... Msg>
void fatal(Msg &&... msg);
template <typename... Msg>
void verbose(Msg &&... msg);
template <typename... Msg>
void log(LogLevel level, Msg &&... msg);

//------------------------------Implementation---------------------------------
template <typename... Msg>
void log(LogLevel level, Msg &&... msg) {
  if (allowed(level)) {
    std::ostringstream oss;
    (oss << ... << std::forward<Msg>(msg));
    logImpl(level, oss.str());
  }
}
template <typename... Msg>
void debug(Msg &&... msg) {
  log(LOG_LEVEL_DEBUG, "DEBUG   :    ", std::forward<Msg>(msg)...);
}
template <typename... Msg>
void info(Msg &&... msg) {
  log(LOG_LEVEL_INFO, "INFO    :    ", std::forward<Msg>(msg)...);
}
template <typename... Msg>
void warn(Msg &&... msg) {
  log(LOG_LEVEL_WARN, "WARN    :    ", std::forward<Msg>(msg)...);
}
template <typename... Msg>
void error(Msg &&... msg) {
  log(LOG_LEVEL_ERROR, "ERROR   :    ", std::forward<Msg>(msg)...);
}
template <typename... Msg>
void fatal(Msg &&... msg) {
  log(LOG_LEVEL_FATAL, "FATAL   :    ", std::forward<Msg>(msg)...);
}
template <typename... Msg>
void verbose(Msg &&... msg) {
  log(LOG_LEVEL_VERBOSE, "VERBOSE :    ", std::forward<Msg>(msg)...);
}

inline bool debugAllowed() { return allowed(LOG_LEVEL_DEBUG); }
inline bool infoAllowed() { return allowed(LOG_LEVEL_INFO); }
inline bool warnAllowed() { return allowed(LOG_LEVEL_WARN); }
inline bool errorAllowed() { return allowed(LOG_LEVEL_ERROR); }
inline bool fatalAllowed() { return allowed(LOG_LEVEL_FATAL); }
inline bool verboseAllowed() { return allowed(LOG_LEVEL_VERBOSE); }

using MafCStr = const char *;
inline constexpr MafCStr constexprPastLastSlash(MafCStr str,
                                                MafCStr last_slash) {
#if defined(_WINDOWS) || defined(WIN32)
  constexpr char slash = '\\';
#else
  constexpr char slash = '/';
#endif
  return *str == '\0'
             ? last_slash
             : *str == slash ? constexprPastLastSlash(str + 1, str + 1)
                             : constexprPastLastSlash(str + 1, last_slash);
}

inline constexpr MafCStr constexprPastLastSlash(MafCStr str) {
  return constexprPastLastSlash(str, str);
}

}  // namespace logging
}  // namespace maf

#define MAF_LOG_LEVEL_SILENCE 0
#define MAF_LOG_LEVEL_DEBUG 1
#define MAF_LOG_LEVEL_INFO 2
#define MAF_LOG_LEVEL_WARN 4
#define MAF_LOG_LEVEL_ERROR 8
#define MAF_LOG_LEVEL_FATAL 16
#define MAF_LOG_LEVEL_VERBOSE 32

#ifndef MAF_MIN_ALLOWED_LOG_LEVEL
#define MAF_MIN_ALLOWED_LOG_LEVEL MAF_LOG_LEVEL_INFO
#endif

#define MAF_LOGGER_DEBUG(...)                                                 \
  do {                                                                        \
    if (maf::logging::debugAllowed()) {                                       \
      maf::logging::debug(__VA_ARGS__, "  --> [[ ", MAF_SHORT_FILE_NAME, ":", \
                          __LINE__, " ]]");                                   \
    }                                                                         \
  } while (false)

#define MAF_LOGGER_WRITE(logtype, ...)                                       \
  do {                                                                       \
    if (maf::logging::logtype##Allowed()) {                                  \
      if (!maf::logging::debugAllowed()) {                                   \
        maf::logging::logtype(__VA_ARGS__);                                  \
      } else {                                                               \
        maf::logging::logtype(__VA_ARGS__, "  --> [[ ", MAF_SHORT_FILE_NAME, \
                              ":", __LINE__, " ]]");                         \
      }                                                                      \
    }                                                                        \
  } while (false)

#if MAF_MIN_ALLOWED_LOG_LEVEL <= MAF_LOG_LEVEL_VERBOSE
#define MAF_LOGGER_VERBOSE(...) MAF_LOGGER_WRITE(verbose, __VA_ARGS__)
#else
#define MAF_LOGGER_VERBOSE(...) while (false)
#endif

#if MAF_MIN_ALLOWED_LOG_LEVEL <= MAF_LOG_LEVEL_INFO
#define MAF_LOGGER_INFO(...) MAF_LOGGER_WRITE(info, __VA_ARGS__)
#else
#define MAF_LOGGER_INFO(...) while (false)
#endif

#if MAF_MIN_ALLOWED_LOG_LEVEL <= MAF_LOG_LEVEL_WARN
#define MAF_LOGGER_WARN(...) MAF_LOGGER_WRITE(warn, __VA_ARGS__)
#else
#define MAF_LOGGER_WARN(...) while (false)
#endif

#if MAF_MIN_ALLOWED_LOG_LEVEL <= MAF_LOG_LEVEL_ERROR
#define MAF_LOGGER_ERROR(...) MAF_LOGGER_WRITE(error, __VA_ARGS__)
#else
#define MAF_LOGGER_ERROR(...) while (false)
#endif

#if MAF_MIN_ALLOWED_LOG_LEVEL <= MAF_LOG_LEVEL_FATAL
#define MAF_LOGGER_FATAL(...) MAF_LOGGER_WRITE(fatal, __VA_ARGS__)
#else
#define MAF_LOGGER_FATAL(...) while (false)
#endif

#define MAF_SHORT_FILE_NAME maf::logging::constexprPastLastSlash(__FILE__)
