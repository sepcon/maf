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
    Timer serverTimer;
    std::shared_ptr<Proxy> proxy;
    Component clientComp;
    clientComp.onMessage<ServiceStatusMsg>([&proxy](const std::shared_ptr<ServiceStatusMsg>& msg){
        thafMsg("Got Service status change update: " << static_cast<int>(msg->newStatus));
        if(msg->newStatus == Availability::Available)
        {
            auto request = std::make_shared<WeatherStusRequest>();
            for (int i = 0; i < 2; ++i) {
                proxy->template sendActionRequestSync<WeatherStusResult>(request, [](const std::shared_ptr<WeatherStusResult>& result){
                    thafMsg("thread " << std::this_thread::get_id() << " get update: " << result->_data.get_the_status());
                });
            }
            request->_data.set_command(1);
            proxy->template sendActionRequest<WeatherStusResult>(request, [](const std::shared_ptr<WeatherStusResult>& result){
                thafMsg("************************************************sending the shutdown request!");
                thafMsg(result->_data.get_the_status());
            });
        }
    });
    clientComp.start([&proxy, &clientComp]{
        proxy = Proxy::createProxy(1);
        thafMsg ("proxy for service 1 ready ! Component  = " << clientComp.getID());
    });

    Component clientComp1;
    std::shared_ptr<Proxy> proxy1;
    clientComp1.onMessage<ServiceStatusMsg>([&proxy1](const std::shared_ptr<ServiceStatusMsg>& msg){
        thafMsg("Got Service status change update: " << static_cast<int>(msg->newStatus));
        if(msg->newStatus == Availability::Available)
        {
            auto request = std::make_shared<WeatherStusRequest>();
            for (int i = 0; i < 2; ++i) {
                proxy1->template sendActionRequestSync<WeatherStusResult>(request, [](const std::shared_ptr<WeatherStusResult>& result){
                    thafMsg("thread " << std::this_thread::get_id() << " get update: " << result->_data.get_the_status());
                });
            }
            request->_data.set_command(1);
            proxy1->template sendActionRequest<WeatherStusResult>(request, [](const std::shared_ptr<WeatherStusResult>& result){
                thafMsg("************************************************sending the shutdown request!");
                thafMsg(result->_data.get_the_status());
            });
        }
    });
    clientComp1.start([&proxy1, &clientComp1]{
        proxy1 = Proxy::createProxy(1);
        thafMsg( "proxy for service 1 ready ! Component  = " << clientComp1.getID());
    });


    Component clientComp2;
    std::shared_ptr<Proxy> proxy2;
    clientComp2.onMessage<ServiceStatusMsg>([&proxy2](const std::shared_ptr<ServiceStatusMsg>& msg){
        thafMsg("Got Service status change update: " << static_cast<int>(msg->newStatus));
        if(msg->newStatus == Availability::Available)
        {
            auto request = std::make_shared<WeatherStusRequest>();
            for (int i = 0; i < 2; ++i) {
                proxy2->template sendActionRequestSync<WeatherStusResult>(request, [](const std::shared_ptr<WeatherStusResult>& result){
                    thafMsg("thread " << std::this_thread::get_id() << " get update: " << result->_data.get_the_status());
                });
            }
            request->_data.set_command(1);
            proxy2->template sendActionRequest<WeatherStusResult>(request, [](const std::shared_ptr<WeatherStusResult>& result){
                thafMsg("************************************************sending the shutdown request!");
                thafMsg(result->_data.get_the_status());
            });
        }
    });
    clientComp2.start([&proxy2, &clientComp2]{
        proxy2 = Proxy::createProxy(1);
        thafMsg( "proxy for service 1 ready ! Component  = " << clientComp2.getID());
    });

    serverComp.onMessage<RequestMessage>([&serverComp, &serverTimer](const std::shared_ptr<RequestMessage>& msg){
        static thread_local int requestCount = 0;
        ++requestCount;
        serverTimer.start(21, [&serverComp]{
            thafMsg(" ******************************************Received command shutdown, total request: " << requestCount);
            serverComp.shutdown();
        });
        auto reqk = msg->getRequestKeeper();
        auto req = reqk->template getRequestContent<WeatherStusRequest>();
        thafMsg("Receiver request from client: " << req->_data.get_client_name());
        auto res = std::make_shared<WeatherStusResult>();
        reqk->reply(res);
//        if(req->_data.get_command() != 0)
//        {
//            thafMsg(" ******************************************Received command shutdown, total request: " << requestCount);
//            serverComp.shutdown();
//        }
    });

    std::shared_ptr<Stub> stub;
    serverComp.start([&stub]{
        stub = Stub::createStub(1);
        thafMsg( "Stub ready as well");


    });
}
int main()
{
    Address addr("com.opswat.client", 0);

    LocalIPCClient::instance().init(addr);
    LocalIPCServer::instance().init(addr);
    test<LocalIPCServiceProxy, LocalIPCServiceStub, IPCClientRequestMsg>();
    test<IAServiceProxy, IAServiceStub, IARequestMesasge>();

    thafMsg("hello world");
	//stl::SyncObject<std::set<ComponentRef>> myList;
	//auto compref1 = std::make_shared< ComponentSyncPtr >();
	//compref1->reset(new Component);
	//myList->insert(compref1);
	//auto compref2 = std::make_shared< ComponentSyncPtr >();
	//compref2->reset(new Component);
	//myList->insert(compref2);
//	std::cin.get();
    return 0;
}
