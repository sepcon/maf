#ifndef CONSOLELOGGER_H
#define CONSOLELOGGER_H

#include "LoggerBase.h"
#include "thaf/patterns/Patterns.h"

namespace thaf {
namespace logging {

DCL_LOGGING_MSG(ConsoleWriteMessage);
class ConsoleLogger :
        public LoggerBase<ConsoleWriteMessage<wchar_t>, ConsoleWriteMessage<char>>
{
private:
    friend class LoggingComponent;
    ConsoleLogger(){}
    virtual void write(const std::string& message, LogLevel level = LogLevel::INFO) override;
    virtual void write(const std::wstring & message, LogLevel level = LogLevel::INFO) override;

};

} // logging
} // thaf
#endif // CONSOLELOGGER_H
