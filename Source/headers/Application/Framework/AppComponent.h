#ifndef APPCOMPONENT_H
#define APPCOMPONENT_H

#include "headers/Application/Framework/Component.h"
namespace thaf {
namespace Threading { class BusyTimer; }
namespace app {

class AppComponent : public Component
{
public:
    AppComponent(bool isMainComponent = false, bool detachedToCurrentThread = true);
	void shutdown() override;
	~AppComponent();
private:
    std::shared_ptr<Threading::BusyTimer> getTimer();
    std::shared_ptr<Threading::BusyTimer> _busyTimer;
    friend class Timer;
};
} // app
} // thaf
#endif // APPCOMPONENT_H
