#include "maf/messaging/client-server/ipc/LocalIPCServiceProxy.h"
#include "maf/messaging/client-server/ipc/LocalIPCServiceStub.h"
#include "maf/messaging/client-server/IAServiceProxy.h"
#include "maf/messaging/client-server/IAServiceStub.h"
#include "maf/messaging/ExtensibleComponent.h"
#include "maf/messaging/Timer.h"
#include "maf/utils/debugging/Debug.h"
#include "maf/utils/TimeMeasurement.h"


using namespace maf::messaging::ipc;
using namespace maf::messaging;
using namespace maf;

#define MAF_ENABLE_DUMP
#include "maf/messaging/client-server/CSContractDefinesBegin.mc.h"
FUNCTION(WeatherStatus)
    using StringList = std::vector<std::string>;
    RESULT_MESSAGE
    (
        (std::string, the_status, "It is going to rain now!"),
        (StringList, list_of_places),
        (StringList, shared_list_of_places)
    )
    REQUEST_MESSAGE
    (
        (std::string, client_name, "This s client"),
        (uint32_t, command, 0)
    )
ENDFUNC(WeatherStatus);

FUNCTION(UpdateSignal)
    using CustomHeader = std::map<std::string, std::string>;
    using String = std::string;
    EVENT_MESSAGE
    (
        (bool,              compliant,           true),
        (bool,              critical,            false),
        (CustomHeader,      headers                   ),
        (String,            cloud_message,       "don't care")
    )
    MESSAGE
    (
        Status,
        (bool, compliant, false),
        (bool, critical, false)
    )
ENDFUNC()

FUNCTION(ComplianceStatus)
    RESULT_MESSAGE
    (
        (bool, compliant, true),
        (bool, critical, false),
        (std::string, cloud_message, "This device is compliant!")
    )
ENDFUNC()

#include <maf/messaging/client-server/CSContractDefinesEnd.mc.h>


template<size_t Size>
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

static std::vector<std::string> listOfPlaces = {"hello", "world"};//std::make_shared<std::vector<std::string>>(createList<10000>());

template<class MessageTrait, class Client, int ServiceID>
struct ClientComponent : public ExtensibleComponent
{
    using Proxy = SCQServiceProxy<MessageTrait, Client>;
    std::shared_ptr<Proxy> _proxy;

    void startTest()
    {
        onMessage<ServiceStatusMsg>([this](const std::shared_ptr<ServiceStatusMsg>& msg) {
            if (msg->newStatus == Availability::Available)
            {
                auto request = WeatherStatus::makeRequest();
                _proxy->template sendStatusChangeRegister<WeatherStatus::Result>
                    ([](const std::shared_ptr<WeatherStatus::Result>& result){
                        mafMsg("thread " << std::this_thread::get_id() << " Got Status update from server: \n" << result->dump());
                    });

                request->set_command(1);
                _proxy->template sendRequest<WeatherStatus::Result>([](const std::shared_ptr<WeatherStatus::Result>& result) {
                    mafMsg(result->dump());
                });
            }
            else
            {

            }
        });

        run(LaunchMode::Async);
    }
    void stopTest()
    {
        _proxy.reset();
        stop();
    }

    void onEntry() override
    {
        _proxy = Proxy::createProxy(ServiceID);
        mafMsg("proxy for service 1 ready ! Component  = " << name());
    }
};


template <class MessageTrait, class Server, int ServiceID>
class ServerComponent : public ExtensibleComponent
{
    using RequestMessage = ClientRequestMessage<MessageTrait>;
    using Stub = SSQServiceStub<MessageTrait, Server>;
    std::shared_ptr<Stub> _stub;
    Timer _serverTimer;
public:
    void startTest(bool detached = true)
    {
        _serverTimer.setCyclic(true);
        onMessage<RequestMessage>([this](const std::shared_ptr<RequestMessage>& msg) {
            auto requestKeeper = msg->getRequestKeeper();
            switch(requestKeeper->getOperationCode())
            {
            case OpCode::Register:
            {
                mafMsg("Client registered to status " << msg->getRequestKeeper()->getOperationID());
                auto msg = WeatherStatus::makeResult();
                msg->set_the_status("Response to register");
                requestKeeper->respond(msg);
            }
                break;
            case OpCode::Request:
            {
                _serverTimer.start(10, [this, requestKeeper] {
                    static thread_local int responseCount = 0;
                    static thread_local long long totalupdateTime = 0;

                    mafMsg("Component id: " << std::this_thread::get_id() << " Timer expired , total request: " << ++responseCount);
                    auto res = std::make_shared<WeatherStatus::Result>();
                    res->set_shared_list_of_places(listOfPlaces);

                    {
                        maf::util::TimeMeasurement updateMeasure{
                            [](long long elapsed) {
                                totalupdateTime += elapsed;
                            }
                        };

                        requestKeeper->update(res);
                    }
                    if(responseCount >= 1000)
                    {
                        mafMsg("Send final update to client");
                        mafMsg("Avarage time to send an update " << " = " << static_cast<double>(totalupdateTime) / static_cast<double>(responseCount) << "ms") ;
                        requestKeeper->respond(res);
                        _serverTimer.stop();
                        this->stop();
                    }
                });
                auto req = msg->template getRequestContent<WeatherStatus::Request>();
                if(req)
                {
                    mafMsg("Receiver request from client: " << req->client_name());
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


        run(detached ? LaunchMode::Async : LaunchMode::AttachToCurrentThread);
    }
    void stopTest()
    {
        _stub.reset();
        this->stop();
    }

    void onEntry() override
    {
        mafMsg("Component is starting");
        _stub = Stub::createStub(ServiceID);
        mafMsg("Stub ready as well");
    }
};


template<class MessageTrait, class Server, class Client, int NClient = 4>
void test()
{
    ClientComponent<MessageTrait, Client, 0> clients;
    clients.startTest();
    ServerComponent<MessageTrait, Server, 0> server;
    server.startTest(false);

    clients.stopTest();
    mafMsg("End test");
}


int main()
{
    maf::util::TimeMeasurement tm([](auto t){
        std::cout << "Total time is: " << t << std::endl;
    });

    Address::create({"comp.opswat.client", 0});
    Address addr("com.opswat.client", 0);
    static constexpr int SERVER_CHECKING_CYCLE_IN_MILLISECONDS = 10;
    LocalIPCClient::instance().init(addr, SERVER_CHECKING_CYCLE_IN_MILLISECONDS);
    LocalIPCServer::instance().init(addr);

    test<IPCMessageTrait, LocalIPCServer, LocalIPCClient, 1>();
    test<IAMessageTrait, IAMessageRouter, IAMessageRouter, 1>();

    return 0;
}

