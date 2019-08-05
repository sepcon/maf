#include "maf/logging/ConsoleLogger.h"
#include "maf/logging/LoggingComponent.h"
#include "maf/messaging/MessageBase.h"
#include <iostream>

namespace maf {
namespace logging {

static std::map<LogLevel, const char* const> LOG_LEVEL_STRINGS =
{
    {LogLevel::NONE,    "[NONE]: "},
    {LogLevel::INFO,    "[INFO]: "},
    {LogLevel::WARNING, "[WARNING]: "},
    {LogLevel::ERROR,   "[ERROR]: "},
    {LogLevel::FATAL,   "[FATAL]: "}
};

void ConsoleLogger::write(const std::string &message, LogLevel level)
{
    auto& os = level >= LogLevel::WARNING ? std::cerr : std::cout;
    os << LOG_LEVEL_STRINGS[level] << message << std::endl;
}

void ConsoleLogger::write(const std::wstring &message, LogLevel level)
{
    auto& os = level >= LogLevel::WARNING ? std::wcerr : std::wcout;
    os << LOG_LEVEL_STRINGS[level] << message << std::endl;
}


} // logging
} // maf
