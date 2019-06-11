#include "headers/Framework/Application/AppComponent.h"
#include "headers/Libs/Threading/Interfaces/BusyTimer.h"
#include "headers/Framework/Logging/LoggingComponent.h"
#include "headers/Framework/Application/Messages.h"

namespace thaf {
namespace app {

AppComponent::AppComponent(bool detachedToCurrentThread) :
    Component(detachedToCurrentThread),
    _logger(logging::LoggingComponent::instance().getLogger(logging::Null))
{
    onMessage<TimeoutMessage>([this](messaging::CMessagePtr<TimeoutMessage> msg) {
        _logger->log("Timer " + std::to_string(msg->timerID) + " expired");
        msg->callback();
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

logging::ILogger *AppComponent::getLogger() const
{
    return _logger;
}

void AppComponent::setLogger(logging::ILogger *logger)
{
    _logger = logger;
}

AppComponent *AppComponent::getThreadLocalInstance()
{
    return static_cast<AppComponent*>(Component::getThreadLocalInstance());
}

AppComponent::~AppComponent()
{
	shutdown();
}

std::shared_ptr<threading::BusyTimer> AppComponent::getTimer()
{
    if(!_busyTimer)
    {
        _busyTimer = std::make_shared<threading::BusyTimer>();
    }
    return _busyTimer;
}

} // app
} // thaf
