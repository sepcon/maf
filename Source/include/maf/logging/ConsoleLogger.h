#ifndef CONSOLELOGGER_H
#define CONSOLELOGGER_H

#include "LoggerBase.h"
#include "maf/patterns/Patterns.h"

namespace maf {
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
} // maf
#endif // CONSOLELOGGER_H
