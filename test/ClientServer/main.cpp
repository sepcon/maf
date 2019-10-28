#include "maf/messaging/client-server/ipc/LocalIPCServiceProxy.h"
#include "maf/messaging/client-server/ipc/LocalIPCServiceStub.h"
#include "maf/messaging/client-server/IAServiceProxy.h"
#include "maf/messaging/client-server/IAServiceStub.h"
#include "maf/messaging/Timer.h"
#include "maf/utils/debugging/Debug.h"
#include "maf/utils/TimeMeasurement.h"
#include "Client.h"
#include "Server.h"


using namespace maf::messaging::ipc;
using namespace maf::messaging;

template<class MessageTrait, class Server, class Client, int NClient = 4>
void test()
{
    using ClientComp = maf::test::ClientComponent<MessageTrait, Client, 0>;
    ClientComp cls[NClient];
    int i = 0;
    for(auto& c : cls)
    {
        c.setName("Client component " + std::to_string(i++));
        c.startTest();
    }

    maf::test::ServerComponent<MessageTrait, Server, 0> server;
    server.setName("Server Component ");
    server.startTest(false);

    for(auto& c : cls)
    {
        c.stopTest();
    }
    mafMsg("End test");
}

#include <iostream>

int main()
{
    maf::debugging::initLogging(maf::debugging::LogLevel::LEVEL_ERROR,
        [](const std::string& msg) {
            std::cout << msg << std::endl;
        },
        [](const std::string& msg) {
            std::cerr << msg << std::endl;
        });
    maf::util::TimeMeasurement tm([](auto t){
        mafMsg("Total time is: " << t);
    });

    Address addr("com.opswat.client", 0);
    LocalIPCServer::instance().init(addr);
    LocalIPCClient::instance().init(addr);

    test<IPCMessageTrait, LocalIPCServer, LocalIPCClient, 2>();
//    test<IAMessageTrait, IAMessageRouter, IAMessageRouter, 1>();

    return 0;
}

