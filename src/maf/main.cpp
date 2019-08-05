#include "maf/utils/debugging/Debug.h"
#include "maf/messaging/client-server/ipc/IPCMessageTrait.h"
#include "maf/utils/serialization/SerializableObject.h"
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
        (std::string, client_name, "This is client"),
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
        _component = std::make_shared<Component>();
//        _component->setName("ServiceClient");
        _component->onMessage<ServiceStatusMsg>([this](const std::shared_ptr<ServiceStatusMsg>& msg) {
            if (msg->newStatus == Availability::Available)
            {
                auto request = std::make_shared<WeatherStatusRequest>();
                _proxy->template sendStatusChangeRegister<WeatherStatusResult>(CSC_OpID_WeatherStatus, [](const std::shared_ptr<WeatherStatusResult>& result){
                    mafMsg("thread " << std::this_thread::get_id() << " Got Status update from server: " << (*result)->get_the_status());
                });

                (*request)->set_command(1);
                _proxy->template sendRequest<WeatherStatusResult>(request, [](const std::shared_ptr<WeatherStatusResult>& result) {
                    auto size = result->props().get_shared_list_of_places() ? result->props().get_shared_list_of_places()->size() : 0;
                    mafMsg("Received result from server with size of listPlaces ----> = " << size);
                    mafMsg((*result)->get_the_status());
                });
            }
        });
        _component->start([this] {
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
        _component = std::make_shared<Component>(detached);
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
                static thread_local int requestCount = 0;
                ++requestCount;
                _serverTimer.start(5, [this] {
                    //				assert(requestCount == (NumberOfRequests + 1) * NClient);
                    mafMsg("Component id: " << std::this_thread::get_id() << " Timer expired , total request: " << requestCount);
                    _component->shutdown();
                });
                auto req = msg->template getRequestContent<WeatherStatusRequest>();
                if(req)
                {
                    mafMsg("Receiver request from client: " << (*req)->get_client_name());
                }
                else
                {
                    mafMsg("Get request with no content, operationID=  " << msg->getRequestKeeper()->getOperationID());
                }

                auto res = std::make_shared<WeatherStatusResult>();
                res->props().set_shared_list_of_places(listOfPlaces);
                for(int i = 0; i < 100; ++i)
                {requestKeeper->update(res);}
                requestKeeper->respond(res);
            }
                break;

            default:
                break;

            }

        });

        //std::shared_ptr<Stub> stub;
        _component->start([this] {
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


template<class Proxy, class Stub, class RequestMessage, int NumberOfRequests = 10, int NClient = 4>
void test()
{
    ClientComponent<Proxy, 0> clients[NClient];
    for (auto i = 0; i < NClient / 4; ++i)
    {
        clients[i].startTest();
    }
    ClientComponent<Proxy, 1> clients1[NClient];
    for (auto i = 0; i < NClient / 4; ++i)
    {
        clients1[i].startTest();
    }

    ClientComponent<Proxy, 2> clients2[NClient];
    for (auto i = 0; i < NClient / 4; ++i)
    {
        clients2[i].startTest();
    }

    ClientComponent<Proxy, 3> clients3[NClient];
    for (auto i = 0; i < NClient / 4; ++i)
    {
        clients3[i].startTest();
    }

    ServerComponent<Stub, RequestMessage, 0> server;
    server.startTest();

    ServerComponent<Stub, RequestMessage, 1> server1;
    server1.startTest();
    
    ServerComponent<Stub, RequestMessage, 2> server2;
    server2.startTest();

    ServerComponent<Stub, RequestMessage, 3> server3;
    server3.startTest(false);

    mafMsg("End test");
}

#include <fstream>

int main()
{
    maf::util::TimeMeasurement tm([](auto t){
        std::cout << "Total time is: " << t << std::endl;
    });
    Address addr("com.opswat.client", 0);
    LocalIPCClient::instance().init(addr, 10);
    LocalIPCServer::instance().init(addr);

    {
        util::TimeMeasurement tm([](long long time) {
            mafMsg("Total test time = " << time << "ms");
        });

        test<LocalIPCServiceProxy, LocalIPCServiceStub, IPCClientRequestMsg>();
        test<IAServiceProxy, IAServiceStub, IARequestMesasge>();

        mafMsg("Program ends!");
    }
//    std::cin.get();
    return 0;
}

