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


using namespace thaf::messaging::ipc;
using namespace thaf::messaging;
using namespace thaf;

class WeatherStusResult : public IPCMessageContentBase
{
public:
    static OpID sOperationID()
    {
        return 1;
    }

    virtual srz::ByteArray  toBytes() noexcept override
    {
        srz::BASerializer sr;
        sr << _data;
        return std::move(sr.mutableBytes());
    }

    virtual void fromBytes(const srz::ByteArray& ba) override
    {
        srz::BADeserializer ds(ba);
        ds >> _data;
    }

    virtual OpID operationID() const override
    {
        return sOperationID();
    }

public:
    mc_sbClass(Data)
        mc_sbProperties((std::string, the_status, "It is going to rain now!"))
    mc_sbClass_end(Data)

        Data _data;
};

class WeatherStusRequest : public IPCMessageContentBase
{
public:
    static OpID sOperationID()
    {
        return 1;
    }

    virtual srz::ByteArray toBytes() noexcept override
    {
        srz::BASerializer sr;
        sr << _data;
        return std::move(sr.mutableBytes());
    }

    virtual void fromBytes(const srz::ByteArray& ba) override
    {
        srz::BADeserializer ds(ba);
        ds >> _data;
    }

    virtual OpID operationID() const override
    {
        return sOperationID();
    }

public:
    mc_sbClass(Data)
        mc_sbProperties((std::string, client_name, "This is client"),
                        (uint32_t, command, 0))
            mc_sbClass_end(Data)

        Data _data;
};

template<class Proxy, class Stub, class RequestMessage>
void test()
{
    Component serverComp(false);
    Component clientComp;

    std::shared_ptr<Proxy> proxy;
    clientComp.onMessage<ServiceStatusMsg>([&proxy](const std::shared_ptr<ServiceStatusMsg>& msg){
        thafMsg("Got Service status change update: " << static_cast<int>(msg->newStatus));
        if(msg->newStatus == Availability::Available)
        {
            auto request = std::make_shared<WeatherStusRequest>();
            for (int i = 0; i < 1000; ++i) {
                proxy->template sendActionRequestSync<WeatherStusResult>(request, [](const std::shared_ptr<WeatherStusResult>& result){
                    thafMsg(result->_data.get_the_status());
                });
            }
            request->_data.set_command(1);
            proxy->template sendActionRequest<WeatherStusResult>(request, [](const std::shared_ptr<WeatherStusResult>& result){
                thafMsg("************************************************sending the shutdown request!");
                thafMsg(result->_data.get_the_status());
            });
        }
    });

    serverComp.onMessage<RequestMessage>([&serverComp](const std::shared_ptr<RequestMessage>& msg){
        static thread_local int requestCount = 0;
        ++requestCount;
        auto reqk = msg->getRequestKeeper();
        auto req = reqk->template getRequestContent<WeatherStusRequest>();
        thafMsg("Receiver request from client: " << req->_data.get_client_name());
        auto res = std::make_shared<WeatherStusResult>();
        reqk->reply(res);
        if(req->_data.get_command() != 0)
        {
            thafMsg(" ******************************************Received command shutdown, total request: " << requestCount);
            serverComp.shutdown();
        }
    });


    clientComp.start([&proxy]{
        proxy = Proxy::createProxy(1);
        std::cout << "proxy for service 1 ready !"  << std::endl;
    });

    std::shared_ptr<Stub> stub;
    serverComp.start([&]{
        stub = Stub::createStub(1);
        std::cout << "Stub ready as well" << std::endl;


    });
}
int main()
{
    Address addr("com.opswat.client", 0);

    LocalIPCClient::instance().init(addr);
    LocalIPCServer::instance().init(addr);
    test<LocalIPCServiceProxy, LocalIPCServiceStub, IPCClientRequestMsg>();
    test<IAServiceProxy, IAServiceStub, IARequestMesasge>();

    thafInfo("hello world");
    return 0;
}
