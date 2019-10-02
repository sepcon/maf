#include "maf/utils/debugging/Debug.h"
#include "maf/messaging/client-server/ipc/IPCMessageTrait.h"
#include "maf/utils/serialization/TupleLikeObject.mc.h"
#include "maf/messaging/client-server/ipc/LocalIPCClient.h"
#include "maf/messaging/client-server/ipc/LocalIPCServer.h"
#include "maf/messaging/client-server/ipc/LocalIPCServiceProxy.h"
#include "maf/messaging/client-server/ipc/LocalIPCServiceStub.h"
#include "maf/messaging/Component.h"
#include "maf/messaging/client-server/IAServiceProxy.h"
#include "maf/messaging/client-server/IAServiceStub.h"
#include "maf/messaging/Timer.h"
#include "maf/messaging/client-server/CSContractDefines.mc.h"
#include "maf/utils/TimeMeasurement.h"


using namespace maf::messaging::ipc;
using namespace maf::messaging;
using namespace maf;

result_object_s(WeatherStatus)
properties((std::string, the_status, "It is going to rain now!"),
           (std::vector<std::string>, list_of_places),
           (std::shared_ptr<std::vector<std::string>>, shared_list_of_places))
result_object_e(WeatherStatus)

request_object_s(WeatherStatus)
properties
(
        (std::string, client_name, "This s client"),
        (uint32_t, command, 0)
        )
request_object_e(WeatherStatus)

result_object_no_props(GetPingResult)

template <size_t Size>
std::vector<std::string> createList()
{
    std::vector<std::string> thelist;
    thelist.reserve(Size);
    for(decltype (Size) i = 0; i < Size; ++i)
    {
        thelist.push_back("hello world");
    }
    return thelist;
}

std::shared_ptr<std::vector<std::string>> listOfPlaces = std::make_shared<std::vector<std::string>>(createList<10000>());

template<class Proxy, int ServiceID>
struct ClientComponent
{
    std::shared_ptr<Proxy> _proxy;
    std::shared_ptr<Component> _component;

    void startTest()
    {
        _component = Component::create();
        _component->onMessage<ServiceStatusMsg>([this](const std::shared_ptr<ServiceStatusMsg>& msg) {
            if (msg->newStatus == Availability::Available)
            {
                auto request = std::make_shared<WeatherStatusRequest>();
                _proxy->template sendStatusChangeRegister<WeatherStatusResult>(CSC_OpID_WeatherStatus, [](const std::shared_ptr<WeatherStatusResult>& result){
                    mafMsg("thread " << std::this_thread::get_id() << " Got Status update from server: " << (*result)->the_status());
                });

                (*request)->set_command(1);
                _proxy->template sendRequest<WeatherStatusResult>(request, [](const std::shared_ptr<WeatherStatusResult>& result) {
                    static thread_local int updateCount = 0;
                    auto size = result->props().shared_list_of_places() ? result->props().shared_list_of_places()->size() : 0;
                    mafMsg("Received result [" << ++updateCount << "] from server with size of listPlaces ----> = " << size);
                    mafMsg((*result)->the_status());
                });
            }
        });

        _component->run(LaunchMode::Async, [this] {
            _proxy = Proxy::createProxy(ServiceID);
            mafMsg("proxy for service 1 ready ! Component  = " << _component->name());
        });
    }
    void stopTest()
    {
        _proxy.reset();
        _component.reset();
    }
};


template <class Stub, class RequestMessage, int ServiceID>
class ServerComponent
{
    std::shared_ptr<Stub> _stub;
    std::shared_ptr<Component> _component;
    Timer _serverTimer;
public:
    void startTest(bool detached = true)
    {
        _serverTimer.setCyclic(true);
        _component = Component::create();
        _component->onMessage<RequestMessage>([this](const std::shared_ptr<RequestMessage>& msg) {
            auto requestKeeper = msg->getRequestKeeper();
            switch(requestKeeper->getOperationCode())
            {
            case OpCode::Register:
            {
                mafMsg("Client registered to status " << msg->getRequestKeeper()->getOperationID());
                auto msg = WeatherStatusResult::create();
                msg->props().set_the_status("Response to register");
                requestKeeper->respond(msg);
            }
                break;
            case OpCode::Request:
            {
                _serverTimer.start(5, [this, requestKeeper] {
                    static thread_local int responseCount = 0;
                    static thread_local long long totalupdateTime = 0;

                    mafMsg("Component id: " << std::this_thread::get_id() << " Timer expired , total request: " << ++responseCount);
                    auto res = std::make_shared<WeatherStatusResult>();
                    res->props().set_shared_list_of_places(listOfPlaces);

                    {
                        maf::util::TimeMeasurement updateMeasure{
                            [](long long elapsed) {
                                totalupdateTime += elapsed;
                            }
                        };

                        requestKeeper->update(res);
                    }
                    if(responseCount >= 100)
                    {
                        mafMsg("Send final update to client");
                        mafMsg("Avarage time to send an update " << " = " << totalupdateTime / responseCount << "ms") ;
                        requestKeeper->respond(res);
                        _serverTimer.stop();
                        _component->stop();
                    }
                });
                auto req = msg->template getRequestContent<WeatherStatusRequest>();
                if(req)
                {
                    mafMsg("Receiver request from client: " << (*req)->client_name());
                }
                else
                {
                    mafMsg("Get request with no content, operationID=  " << msg->getRequestKeeper()->getOperationID());
                }
            }
                break;

            default:
                break;

            }

        });

        //std::shared_ptr<Stub> stub;
        _component->run(detached ? LaunchMode::Async : LaunchMode::AttachToCurrentThread, [this] {
            _stub = Stub::createStub(ServiceID);
            mafMsg("Stub ready as well");
        });
    }
    void stopTest()
    {
        _stub.reset();
        _component.reset();
    }
};


template<class Proxy, class Stub, class RequestMessage, int NClient = 4>
void test()
{
    ClientComponent<Proxy, 0> clients;
    clients.startTest();
    ServerComponent<Stub, RequestMessage, 0> server;
    server.startTest(false);

    clients.stopTest();
    mafMsg("End test");
}

#include <fstream>
#include <maf/messaging/ExtensibleComponent.h>
#include <maf/messaging/CompThread.h>

struct Runner
{
    void run()
    {
        using namespace std::chrono_literals;
		_tm = new maf::util::TimeMeasurement{ [](long long mls) { std::cout << "time for 10 expiration = " << mls << std::endl; } };
        _timer.setCyclic(true);
        _timer.start(100, [this]{
            static thread_local int  i = 0;
            i++;
//            mafMsg("Timer expiered " << i++);
            if(i == 10)
            {
                Component::getActiveSharedPtr()->postMessage<ShutdownMessage>();
				i = 0;
				delete _tm;
				_tm = nullptr;
            }
        });
        for(int i = 0; i < 10; ++i)
        {
//            mafMsg("one more output " << i);
            std::this_thread::sleep_for(1ms);
        }
    }
	maf::util::TimeMeasurement* _tm;
    Timer _timer = true;
};

class MyComp : public ExtensibleComponent
{
public:
    MyComp() = default;
    ~MyComp() { }
    void onEntry() override
    {
        onSignal<maf::messaging::ShutdownMessage>([this]{
            stop();
            if(_th.joinable())
            {
                _th.join();
            }
        });

        _th = CompThread{ &Runner::run, &_myRunner};
        _th.start();
    }
    void onExit() override
    {
//        mafMsg("Component stops!");
    }

    CompThread _th;
    Runner _myRunner;
};

int main()
{
    maf::util::TimeMeasurement tm([](auto t){
        std::cout << "Total time is: " << t << std::endl;
    });

	for (int i = 0; i < 1; ++i)
	{
		MyComp comp;
		comp.run(LaunchMode::AttachToCurrentThread);
	}

//    Address addr("com.opswat.client", 0);
//    static constexpr int SERVER_CHECKING_CYCLE_IN_MILLISECONDS = 10;
//    LocalIPCClient::instance().init(addr, SERVER_CHECKING_CYCLE_IN_MILLISECONDS);
//    LocalIPCServer::instance().init(addr);

//    {
//        util::TimeMeasurement tm([](long long time) {
//            mafMsg("Total test time = " << time << "ms");
//        });

//        test<LocalIPCServiceProxy, LocalIPCServiceStub, IPCClientRequestMsg, 1>();
////        test<IAServiceProxy, IAServiceStub, IARequestMesasge>();

//        mafMsg("Program ends!");
//    }
//    std::cin.get();
    return 0;
}

