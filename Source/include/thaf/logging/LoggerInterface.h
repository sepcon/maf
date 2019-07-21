#ifndef LOGGERINTERFACE_H
#define LOGGERINTERFACE_H

#include <string>
#include "thaf/messaging/MessageBase.h"

namespace thaf {
namespace logging {

enum class LogLevel : unsigned char
{
    NONE,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

class ILogger
{
public:
    virtual void log(const std::string& message, LogLevel level = LogLevel::INFO) = 0;
    virtual void log(const std::wstring& message, LogLevel level = LogLevel::INFO) = 0;
    virtual ~ILogger(){}
};


} // app
} // thaf
#endif // LOGGERINTERFACE_H
