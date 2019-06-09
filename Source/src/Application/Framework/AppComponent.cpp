#include "headers/Application/Framework/AppComponent.h"
#include "headers/Threading/Interfaces/BusyTimer.h"

namespace thaf {
namespace app {

AppComponent::AppComponent(bool isMainComponent, bool detachedToCurrentThread) :
    Component (isMainComponent, detachedToCurrentThread)
{
    match<TimeoutMessage>([](Messaging::CMessagePtr msg) {
        auto timeoutMsg = std::static_pointer_cast<TimeoutMessage>(msg);
        if (timeoutMsg)
        {
            timeoutMsg->callback();
        }
    });
}

void AppComponent::shutdown()
{
	if (_busyTimer)
	{
		_busyTimer.reset();
	}
	Component::shutdown();
}

AppComponent::~AppComponent()
{
	shutdown();
}

std::shared_ptr<Threading::BusyTimer> AppComponent::getTimer()
{
    if(!_busyTimer)
    {
        _busyTimer = std::make_shared<Threading::BusyTimer>();
    }
    return _busyTimer;
}

} // app
} // thaf
