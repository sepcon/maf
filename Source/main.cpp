#include <iostream>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include "headers/Framework/Application/AppComponent.h"
#include "headers/Framework/Application/Timer.h"
#include "headers/Framework/Logging/LoggingComponent.h"
#include "headers/Libs/Patterns/Patterns.h"
#include "headers/Framework/Application/Application.h"

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


void startApplication()
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

#include "headers/Libs/Utils/Serialization/SerializableObject.h"

using namespace thaf::srz;

enum class FunctionID : uint32_t
{
    SayHello = 1,
    SayGoodBye = 100
};

SERIALIZABLE_OBJECT(SubOject)
    PROPERTIES_MAP
    (
        (std::string, Properties),
        (uint32_t, Index)
    )
SERIALIZABLE_OBJECT_END(SubOject)

SERIALIZABLE_OBJECT(TheObject)
    PROPERTIES_MAP
    (
        ( std::string, Name ),
        ( FunctionID, FuncID ),
        ( std::shared_ptr<std::string>, Action ),
        ( std::string, Address ),
        ( SubOject, Sub ),
        ( std::vector<uint32_t>, ListOfIndex )
    )
SERIALIZABLE_OBJECT_END(TheObject)

int main()
{
    const std::string dest = "D:\\binary.dat"; //"/home/sepcon/Desktop/binary.dat"; //
    auto startTime = system_clock::now();
//    try
//    {

        std::ofstream writer(dest, std::ios::binary);
        StreamSerializer sr(writer);
        for(int i = 0; i < 100; ++i)
        {
            SubOject sub{"Helo", 10};
            TheObject obj; //{ "sdfsdf", FunctionID::SayHello, nullptr, "sdfafqer", SubOject{"Helo", 10} , std::vector<uint32_t>{} };
            obj.setName("Hello");
            obj.setListOfIndex({1, 2, 3});
            std::string nestring = "Nguyen van con from ha noi";
            sr << nestring << obj;
        }

        sr.flush();

        std::ifstream reader(dest, std::ios_base::binary);
        StreamDeserializer dsrz(reader, 500);
        int i = 0;
        std::istringstream s;

        while(!dsrz.exhaust())
        {
            ++i;
            TheObject to1;
            std::string nestring1;
            dsrz >> nestring1 >> to1;
            std::cout << i << ". name = " << to1.getName()
                      << " type = " << static_cast<uint32_t>(to1.getFuncID())
                      << " action = " << (to1.getAction() ? *to1.getAction() : "nul")
                      << " address = " << to1.getAddress()
                      << " sub = " << to1.getSub().getProperties()
                      << " New String1 = " << nestring1;
            for(const auto& i : to1.getListOfIndex())
            {
                std::cout << i << ", ";
            }
            std::cout << std::endl;
        }
//    }
//    catch (const std::exception& e)
//    {
//        std::cout << "Dammaged Serializtion data with exception: " << e.what() << std::endl;
//    }

    std::cout << "total time: " << std::chrono::duration_cast<milliseconds>(system_clock::now() - startTime).count() << std::endl;

    //    thaf::app::Timer t;
//    t.setCyclic(true);
//    t.start(1000, [] { LOGG("hELLO---------------"); });
//    startApplication();
    return 0;
}
