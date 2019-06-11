#ifndef APPCOMPONENT_H
#define APPCOMPONENT_H

#include "headers/Framework/Application/Component.h"
#include "headers/Framework/Logging/LoggerInterface.h"

namespace thaf {
namespace threading { class BusyTimer; }

namespace app {

class AppComponent : public Component
{
public:
    AppComponent(bool detachedToCurrentThread = true);
    ~AppComponent();
    void shutdown();
    logging::ILogger *getLogger() const;
    void setLogger(logging::ILogger *logger);
    static AppComponent *getThreadLocalInstance();

private:
    std::shared_ptr<threading::BusyTimer> getTimer();
    std::shared_ptr<threading::BusyTimer> _busyTimer;
    logging::ILogger* _logger;

    friend class Timer;
};
} // app
} // thaf
#endif // APPCOMPONENT_H
