#include "maf/Application/AppComponent.h"
#include "maf/Application/Timer.h"
#include "maf/Logging/LoggingComponent.h"
#include "maf/Patterns/Patterns.h"
#include "maf/Application/Application.h"
#include <sstream>

using namespace std::chrono;
using namespace maf::app;
using namespace maf::messaging;
using namespace maf::logging;

#define LOGG(exp) { \
    std::ostringstream oss;  \
    oss << exp; \
    LoggingComponent::instance().getLogger(maf::logging::Console)->log(oss.str()); \
    }

class PrintLogMessage : public InternalMessage {};
class StopTimer : public InternalMessage{};

class ThreatDetector : public maf::app::AppComponent
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
            if(_t.running())
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
    maf::app::Timer _t;
};


static void startApplication()
{
    Timer timer;
    Timer timer1;
    LoggingComponent::instance().init();
    ThreatDetector _threatDetector;
    Application::instance().setLogger(LoggingComponent::instance().getLogger(maf::logging::Console));
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

void maf_fwapp_runTest()
{
    startApplication();
}
