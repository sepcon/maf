#include <iostream>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <string>

#include "headers/Application/Framework/AppComponent.h"
#include "headers/Application/Framework/Timer.h"

using namespace std::chrono;

static std::mutex _coutmt;
#define LOGG(exp) \
{ \
std::lock_guard<std::mutex> lock(_coutmt); \
std::cout << exp << std::endl; \
}



using namespace thaf::app;
using namespace thaf::Messaging;


class PrintLogMessage : public InternalMessage
{
};

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
        match<ScanThreatRequestMsg>([this](CMessagePtr msg){ 
			this->scanThreat();
			if (++count == 3)
			{
				std::static_pointer_cast<ScanThreatRequestMsg>(msg)->_app->shutdown();
			}
			});
        match<ServerRequestScanThreatMsg>([this](CMessagePtr msg){
            this->scanThreat(std::static_pointer_cast<ServerRequestScanThreatMsg>(msg)->Command);
            if(_t.isRunning())
            {
                LOGG( "Timer _t of threat detector is still running!" );
            }
            else {
                _t.start(5000, []{
                    LOGG( "Timer expired!" );
                }, true);
            }
        });
    }

private:
	int count = 0;
    void scanThreat()
    {
        LOGG(std::this_thread::get_id() << "Do scan threat!" );
        if(Component::getMainComponent())
        {
            Component::getMainComponent()->postMessage<PrintLogMessage>();
        }
    }
    void scanThreat(const std::string& command)
    {
        LOGG( "Do scan threat with command " << command );
    }
    thaf::app::Timer _t;
};

class Application : AppComponent
{
public:
	Application() : AppComponent(false, true)
	{
		match<PrintLogMessage>([](CMessagePtr) { LOGG("Logging new message!"); });
	}
	void start()
	{
		_threatDetector.start();
		AppComponent::start([this](CMessagePtr) {
			Timer timer;
			timer.start(100, [this] {_threatDetector.postMessage<ThreatDetector::ScanThreatRequestMsg>(static_cast<AppComponent*>(this)); }, true);
			});
	}

private:
	ThreatDetector _threatDetector;
};

int main()
{
	std::vector<Application*> apps;
	for (int i = 0; i < 10; ++i)
	{
		apps.emplace_back(new Application());
	}
	for (auto& app : apps)
	{
		app->start();
	}
	AppComponent app(true, true);
	app.start();
	std::cin.get();
    return 0;
}
