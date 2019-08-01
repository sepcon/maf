#include "thaf/utils/debugging/Debug.h"
#include "thaf/messaging/client-server/ipc/IPCMessageTrait.h"
#include "thaf/utils/serialization/SerializableObject.h"
#include "thaf/messaging/client-server/ipc/LocalIPCClient.h"
#include "thaf/messaging/client-server/ipc/LocalIPCServer.h"
#include "thaf/messaging/client-server/ipc/LocalIPCServiceProxy.h"
#include "thaf/messaging/client-server/ipc/LocalIPCServiceStub.h"
#include "thaf/messaging/Component.h"
#include "thaf/messaging/client-server/IAServiceProxy.h"
#include "thaf/messaging/client-server/IAServiceStub.h"
#include "thaf/messaging/Timer.h"
#include "thaf/messaging/client-server/CSContractDefines.mc.h"
#include "thaf/utils/TimeMeasurement.h"


using namespace thaf::messaging::ipc;
using namespace thaf::messaging;
using namespace thaf;

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

    void startTest(ClientBase* client)
    {
        _component = std::make_shared<Component>();
//        _component->setName("ServiceClient");
        _component->onMessage<ServiceStatusMsg>([this](const std::shared_ptr<ServiceStatusMsg>& msg) {
            if (msg->newStatus == Availability::Available)
            {
                auto request = std::make_shared<WeatherStatusRequest>();
                _proxy->template sendStatusChangeRegister<WeatherStatusResult>(CSC_OpID_WeatherStatus, [](const std::shared_ptr<WeatherStatusResult>& result){
                    thafMsg("thread " << std::this_thread::get_id() << " Got Status update from server: " << (*result)->get_the_status());
                });

                (*request)->set_command(1);
                _proxy->template sendRequest<WeatherStatusResult>(request, [](const std::shared_ptr<WeatherStatusResult>& result) {
                    auto size = result->props().get_shared_list_of_places() ? result->props().get_shared_list_of_places()->size() : 0;
                    thafMsg("Received result from server with size of listPlaces ----> = " << size);
                    thafMsg((*result)->get_the_status());
                });
            }
        });
        _component->start([this, client] {
            _proxy = client->createProxy<Proxy>(ServiceID);
            thafMsg("proxy for service 1 ready ! Component  = " << _component->name());
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
    void startTest(ServerBase* server, bool detached = true)
    {
        _component = std::make_shared<Component>(detached);
        _component->onMessage<RequestMessage>([this](const std::shared_ptr<RequestMessage>& msg) {
            auto requestKeeper = msg->getRequestKeeper();
            switch(requestKeeper->getOperationCode())
            {
            case OpCode::Register:
            {
                thafMsg("Client registered to status " << msg->getRequestKeeper()->getOperationID());
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
                    thafMsg("Component id: " << std::this_thread::get_id() << " Timer expired , total request: " << requestCount);
                    _component->shutdown();
                });
                auto req = msg->template getRequestContent<WeatherStatusRequest>();
                if(req)
                {
                    thafMsg("Receiver request from client: " << (*req)->get_client_name());
                }
                else
                {
                    thafMsg("Get request with no content, operationID=  " << msg->getRequestKeeper()->getOperationID());
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
        _component->start([this, server] {
            _stub = server->createStub<Stub>(ServiceID);
            thafMsg("Stub ready as well");
        });
    }
    void stopTest()
    {
        _stub.reset();
        _component.reset();
    }
};


template<class Proxy, class Stub, class RequestMessage, int NumberOfRequests = 10, int NClient = 4>
void test(ClientBase* clientBase, ServerBase* serverBase)
{
    ClientComponent<Proxy, 0> clients[NClient];
    for (auto i = 0; i < NClient / 4; ++i)
    {
        clients[i].startTest(clientBase);
    }
    ClientComponent<Proxy, 1> clients1[NClient];
    for (auto i = 0; i < NClient / 4; ++i)
    {
        clients1[i].startTest(clientBase);
    }

    ClientComponent<Proxy, 2> clients2[NClient];
    for (auto i = 0; i < NClient / 4; ++i)
    {
        clients2[i].startTest(clientBase);
    }

    ClientComponent<Proxy, 3> clients3[NClient];
    for (auto i = 0; i < NClient / 4; ++i)
    {
        clients3[i].startTest(clientBase);
    }

    ServerComponent<Stub, RequestMessage, 0> server;
    server.startTest(serverBase);

    ServerComponent<Stub, RequestMessage, 1> server1;
    server1.startTest(serverBase);
    
    ServerComponent<Stub, RequestMessage, 2> server2;
    server2.startTest(serverBase);

    ServerComponent<Stub, RequestMessage, 3> server3;
    server3.startTest(serverBase, false);

    thafMsg("End test");
}

#include <fstream>

int main()
{
    thaf::util::TimeMeasurement tm([](auto t){
        std::cout << "Total time is: " << t << std::endl;
    });
    Address addr("com.opswat.client", 0);
    LocalIPCClient c; c.init(addr, 10);
    LocalIPCServer s; s.init(addr);

    {
        util::TimeMeasurement tm([](long long time) {
            thafMsg("Total test time = " << time << "ms");
        });

        test<LocalIPCServiceProxy, LocalIPCServiceStub, IPCClientRequestMsg>(&c, &s);
        test<IAServiceProxy, IAServiceStub, IARequestMesasge>(&(IAMessageRouter::instance()), &(IAMessageRouter::instance()));

        thafMsg("Program ends!");
    }
//    std::cin.get();
    return 0;
}

