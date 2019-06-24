#include "headers/Framework/Application/AppComponent.h"
#include "headers/Framework/Application/Timer.h"
#include "headers/Framework/Logging/LoggingComponent.h"
#include "headers/Libs/Patterns/Patterns.h"
#include "headers/Framework/Application/Application.h"
#include <iostream>
#include <sstream>

using namespace std::chrono;
using namespace thaf::app;
using namespace thaf::messaging;
using namespace thaf::logging;

#define LOGG(exp) { \
    std::ostringstream oss;  \
    oss << exp; \
    LoggingComponent::instance().getLogger(thaf::logging::Console)->log(oss.str()); \
    }

class PrintLogMessage : public InternalMessage {};
class StopTimer : public InternalMessage{};

class ThreatDetector : public thaf::app::AppComponent
{
public:
    class ScanThreatRequestMsg : public InternalMessage {
    public:
        AppComponent* _app;
        ScanThreatRequestMsg(AppComponent* app) : _app(app)
        {

        }
    };
    class ServerRequestScanThreatMsg : public InternalMessage {
    public:
        ServerRequestScanThreatMsg(const std::string& command) : Command(command)
        {
        }
        std::string Command;
    };

    ThreatDetector()
    {
        onSignal<ScanThreatRequestMsg>([this](){
            this->scanThreat();
            if (++count >= 100)
            {
                Application::instance().postMessage<ShutdownMessage>();
            }
        });
        onMessage<ServerRequestScanThreatMsg>([this](CMessageBasePtr msg){
            this->scanThreat(std::static_pointer_cast<ServerRequestScanThreatMsg>(msg)->Command);
            if(_t.isRunning())
            {
                LOGG("Timer is still running");
            }
            else {
                _t.start(5000, []{
                    LOGG( "Timer expired!" );
                });
            }
        });
    }

private:
    int count = 0;
    void scanThreat()
    {
        LOGG(std::this_thread::get_id() << "Do scan threat!" );
    }
    void scanThreat(const std::string& command)
    {
        LOGG( "Do scan threat with command " << command );
    }
    thaf::app::Timer _t;
};


static void startApplication()
{
    Timer timer;
    Timer timer1;
    LoggingComponent::instance().init();
    ThreatDetector _threatDetector;
    Application::instance().setLogger(LoggingComponent::instance().getLogger(thaf::logging::Console));
    Application::instance().onSignal<PrintLogMessage>([]{ LOGG("Logging new message!"); });
    Application::instance().onSignal<StopTimer>([&timer1, &timer]{ timer1.stop(); timer.stop(); });
    auto startTime = system_clock::now();
    _threatDetector.start();
    Application::instance().start([&timer, &_threatDetector]{
        timer.setCyclic(true);
        timer.start(0, [&_threatDetector] {_threatDetector.postMessage<ThreatDetector::ScanThreatRequestMsg>(static_cast<AppComponent*>(&Application::instance())); });
    });

    LOGG(std::chrono::duration_cast<milliseconds>(system_clock::now() - startTime).count());
}

void thaf_fwapp_runTest()
{
    startApplication();
}
