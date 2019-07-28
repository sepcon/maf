#include "thaf/utils/debugging/Debug.h"
#include "thaf/messaging/ipc/IPCMessageTrait.h"
#include "thaf/utils/serialization/SerializableObject.h"
#include "thaf/messaging/ipc/LocalIPCClient.h"
#include "thaf/messaging/ipc/LocalIPCServer.h"
#include "thaf/messaging/ipc/LocalIPCServiceProxy.h"
#include "thaf/messaging/ipc/LocalIPCServiceStub.h"
#include "thaf/messaging/Component.h"
#include "thaf/messaging/client-server/IAServiceProxy.h"
#include "thaf/messaging/client-server/IAServiceStub.h"
#include "thaf/messaging/Timer.h"
#include "thaf/messaging/ipc/IPCMsgDefinesMacros.h"
#include "thaf/utils/TimeMeasurement.h"


using namespace thaf::messaging::ipc;
using namespace thaf::messaging;
using namespace thaf;

result_object_s(WeatherStatus)
    properties((std::string, the_status, "It is going to rain now!"),
               (std::vector<std::string>, list_of_places))
result_object_e(WeatherStatus)

request_object_s(WeatherStatus)
properties
(
        (std::string, client_name, "This is client"),
        (uint32_t, command, 0)
)
request_object_e(WeatherStatus)


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

std::vector<std::string> listOfPlaces = createList<1000>();

template<class Proxy>
struct ClientComponent
{
    std::shared_ptr<Proxy> _proxy;
    std::shared_ptr<Component> _component;
    void startTest()
    {
        _component = std::make_shared<Component>();
        _component->onMessage<ServiceStatusMsg>([this](const std::shared_ptr<ServiceStatusMsg>& msg) {
            if (msg->newStatus == Availability::Available)
            {
                auto request = std::make_shared<WeatherStatusRequest>();
                for (int i = 0; i < 10; ++i)
                {
                    {
                        util::TimeMeasurement t([](auto time) {
                            thafMsg("Time to execute a requestsync = " << time);
                        });
                        auto response = _proxy->template sendActionRequestSync<WeatherStatusResult>(request, 4000);
                        if(response)
                        {
                            thafMsg("thread " << std::this_thread::get_id() << " get update: " << (*response)->get_the_status());
                        }
                    }
//                    _proxy->template sendActionRequestSync<WeatherStatusResult>(request, [](const std::shared_ptr<WeatherStatusResult>& result) {
//                        thafMsg("thread " << std::this_thread::get_id() << " get update: " << (*result)->get_the_status());
//                        });
                }
                (*request)->set_command(1);
                _proxy->template sendActionRequestSync<WeatherStatusResult>(request, [](const std::shared_ptr<WeatherStatusResult>& result) {
                    thafMsg("************************************************sending the shutdown request!");
                    thafMsg((*result)->get_the_status());
                    });
            }
            });
        _component->start([this] {
            _proxy = Proxy::createProxy(1);
            thafMsg("proxy for service 1 ready ! Component  = " << _component->getID());
            });
    }
    void stopTest()
    {
        _proxy.reset();
        _component.reset();
    }
};


template <class Stub, class RequestMessage>
class ServerComponent
{
	std::shared_ptr<Stub> _stub;
	std::shared_ptr<Component> _component;
    Timer _serverTimer;
public:
	void startTest()
	{
        _component = std::make_shared<Component>(false);
        _component->onMessage<RequestMessage>([this](const std::shared_ptr<RequestMessage>& msg) {
			static thread_local int requestCount = 0;
			++requestCount;
            _serverTimer.start(15, [this] {
//				assert(requestCount == (NumberOfRequests + 1) * NClient);
				thafMsg(" ******************************************Received command shutdown, total request: " << requestCount);
                _component->shutdown();
				});
			auto reqk = msg->getRequestKeeper();
			auto req = reqk->template getRequestContent<WeatherStatusRequest>();
			thafMsg("Receiver request from client: " << (*req)->get_client_name());
			auto res = std::make_shared<WeatherStatusResult>();
            res->props().set_list_of_places(listOfPlaces);
			reqk->reply(res);
			});

		//std::shared_ptr<Stub> stub;
        _component->start([this] {
            _stub = Stub::createStub(1);
			thafMsg("Stub ready as well");
			});
	}
    void stopTest()
    {
        _stub.reset();
        _component.reset();
    }
};


template<class Proxy, class Stub, class RequestMessage, int NumberOfRequests = 10, int NClient = 1>
void test()
{
    ClientComponent<Proxy> clients[NClient];
    for (auto i = 0; i < NClient; ++i)
    {
        clients[i].startTest();
    }

    //auto clientComp1 = createClientComponent<Proxy>();
    //auto clientComp2 = createClientComponent<Proxy>();
    ServerComponent<Stub, RequestMessage> server;
    server.startTest();
//    LocalIPCClient::instance().deinit();
//    LocalIPCServer::instance().deinit();
//    IAMessageRouter::instance().deinit();
//    server.stopTest();
//    for(auto& cl : clients)
//    {
//        cl.stopTest();
//    }
	thafMsg("End test");
}
int main()
{
    Address addr("com.opswat.client", 0);

    LocalIPCClient::instance().init(addr);
    LocalIPCServer::instance().init(addr);

    test<LocalIPCServiceProxy, LocalIPCServiceStub, IPCClientRequestMsg>();
//    test<IAServiceProxy, IAServiceStub, IARequestMesasge>();

    thafMsg("Program ends!");
	std::cin.get();
    return 0;
}

