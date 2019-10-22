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
    for(auto& c : cls)
    {
        c.startTest();
    }

    maf::test::ServerComponent<MessageTrait, Server, 0> server;
    server.startTest(false);

    for(auto& c : cls)
    {
        c.stopTest();
    }
    mafMsg("End test");
}

int main()
{
    maf::util::TimeMeasurement tm([](auto t){
        std::cout << "Total time is: " << t << std::endl;
    });

    Address addr("com.opswat.client", 0);
    static constexpr int SERVER_CHECKING_CYCLE_IN_MILLISECONDS = 10;
    LocalIPCClient::instance().init(addr, SERVER_CHECKING_CYCLE_IN_MILLISECONDS);
    LocalIPCServer::instance().init(addr);


//    test<IPCMessageTrait, LocalIPCServer, LocalIPCClient, 2>();
    test<IAMessageTrait, IAMessageRouter, IAMessageRouter, 1>();

    return 0;
}

