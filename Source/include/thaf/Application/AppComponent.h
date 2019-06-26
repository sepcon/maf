#ifndef APPCOMPONENT_H
#define APPCOMPONENT_H

#include "thaf/Application/Component.h"
#include "thaf/Logging/LoggerInterface.h"

namespace thaf {
namespace threading { class TimerManager; }

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
    std::shared_ptr<threading::TimerManager> getTimeManager();
    std::shared_ptr<threading::TimerManager> _timerMgr;
    logging::ILogger* _logger;

    friend class Timer;
};
} // app
} // thaf
#endif // APPCOMPONENT_H
