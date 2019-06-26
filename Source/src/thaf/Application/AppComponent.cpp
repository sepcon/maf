#include "thaf/Application/AppComponent.h"
#include "thaf/Threading/TimerManager.h"
#include "thaf/Logging/LoggingComponent.h"
#include "thaf/Application/BasicMessages.h"

namespace thaf {
namespace app {

AppComponent::AppComponent(bool detachedToCurrentThread) :
    Component(detachedToCurrentThread),
    _logger(logging::LoggingComponent::instance().getLogger(logging::Null))
{
    onMessage<TimeoutMessage>([this](messaging::CMessagePtr<TimeoutMessage> msg) {
        _logger->log("Timer " + std::to_string(msg->timerID) + " expired");
        msg->execute();
    });
    onMessage<CallbackExcMsg>([](messaging::CMessagePtr<CallbackExcMsg> msg) {
        msg->execute();
    });
}

void AppComponent::shutdown()
{
	if (_timerMgr)
	{
		_timerMgr.reset();
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

std::shared_ptr<threading::TimerManager> AppComponent::getTimeManager()
{
    if(!_timerMgr)
    {
        _timerMgr = std::make_shared<threading::TimerManager>();
    }
    return _timerMgr;
}

} // app
} // thaf
