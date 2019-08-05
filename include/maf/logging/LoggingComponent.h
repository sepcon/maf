#ifndef LOGGINGCOMPONENT_H
#define LOGGINGCOMPONENT_H

#include "maf/messaging/Component.h"
#include "maf/patterns/Patterns.h"
#include "LoggerInterface.h"

namespace maf {
namespace logging {

using LoggerDestinationName = std::string;
const char* const Console = "console";
const char* const Null = "null";

class LoggingComponent : public messaging::Component, public pattern::SingletonObject<LoggingComponent>
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
} // maf
#endif // LOGGINGCOMPONENT_H
