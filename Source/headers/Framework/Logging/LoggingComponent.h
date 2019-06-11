#ifndef LOGGINGCOMPONENT_H
#define LOGGINGCOMPONENT_H

#include "headers/Framework/Application/Component.h"
#include "headers/Libs/Patterns/Patterns.h"
#include "LoggerInterface.h"

namespace thaf {
namespace logging {

using LoggerDestinationName = std::string;
const char* const Console = "console";
const char* const Null = "null";

class LoggingComponent : public app::Component, public pattern::SingletonObject<LoggingComponent>
{
public:
    LoggingComponent(Invisible);
	void init();
    ILogger *getLogger(LoggerDestinationName dest);
    void registerLogger(LoggerDestinationName dest, ILogger* pLogger);

private:
    std::map<std::string, ILogger*> _loggers;
    std::mutex m;
};
} // app
} // thaf
#endif // LOGGINGCOMPONENT_H
