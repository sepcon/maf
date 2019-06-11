#include <iostream>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <sstream>

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

enum FunctionID
{
    SayHello = 1,
    SayGoodBye = 100
};


SERIALIZABLE_OBJECT(SubOject)
    HAS_PROPERTIES(Properties)
    WITH_RESPECTIVE_TYPES(std::wstring)
    DEFINE_GET_SET_METHODS(std::wstring, Properties)
SERIALIZABLE_OBJECT_END(SubOject)

SERIALIZABLE_OBJECT(TheObject)
    HAS_PROPERTIES(Name, FuncID, Action, Address, Sub)
    WITH_RESPECTIVE_TYPES(std::wstring, FunctionID, std::wstring, std::wstring, SubOject)
    DEFINE_GET_SET_METHODS(std::wstring, Name)
    DEFINE_GET_SET_METHODS(FunctionID, FuncID)
    DEFINE_GET_SET_METHODS(std::wstring, Action)
    DEFINE_GET_SET_METHODS(std::wstring, Address)
    DEFINE_GET_SET_METHODS(SubOject, Sub)
SERIALIZABLE_OBJECT_END(TheObject)


#include <fstream>


int main()
{
    const std::string dest = "/home/sepcon/Desktop/binary.dat"; //"D:\\binary.dat"; //
    auto startTime = std::chrono::system_clock::now();
    try
    {
//        std::ofstream writer(dest, std::ios::binary);
//        StreamSerializer sr(writer);
//        for(int i = 0; i < 1000; ++i)
//        {
//            TheObject obj;
//            obj.setName(L"Hello world");
//            obj.setAction(L"The action");
//            obj.setFuncID(SayHello);
//            obj.setAddress(L"Vietname quoc oai");
//            obj.setSub(SubOject(L"hallu world"));

//            std::wstring newString = L"Nguyen van con from ha noi";
//            sr << newString << obj;
//        }
//        sr.close();
//        writer.close();

        std::ifstream reader(dest, std::ios::binary);
        StreamDeserializer dsrz(reader, 500);
        int i = 0;

        while(!dsrz.exhaust())
        {
            ++i;
            TheObject to1;
            std::wstring newString1;
            dsrz >> newString1 >> to1;
//            std::cout << i << ". name = " << to1.getName()
//                      << " type = " << to1.getFuncID()
//                      << " action = " << to1.getAction()
//                      << " address = " << to1.getAddress()
//                      << " sub = " << to1.getSub().getProperties()
//                      << " New String1 = " << newString1 << std::endl;

        }
    }
    catch (const std::exception& e)
    {
        std::cout << "Dammaged Serializtion data with exception: " << e.what() << std::endl;
    }

    std::cout << "total time: " << std::chrono::duration_cast<milliseconds>(system_clock::now() - startTime).count() << std::endl;
//    thaf::app::Timer t;
//    t.setCyclic(true);
//    t.start(1000, [] { LOGG("hELLO---------------"); });
//    startApplication();
    return 0;
}
