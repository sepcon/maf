#include "maf/messaging/client-server/IAServiceStub.h"
#include <maf/messaging/client-server/SerializableMessageTrait.h>
#include "maf/messaging/Timer.h"
#include <maf/logging/Logger.h>
#include "maf/utils/TimeMeasurement.h"
#include "Client.h"
#include "Server.h"


using namespace maf::messaging::ipc;
using namespace maf::messaging;

template<class MessageTrait, int NClient = 4>
void test(const ConnectionType& connectionType, const Address& addr)
{
    static constexpr ServiceID sid = 0;
    using ClientComp = maf::test::ClientComponent<MessageTrait, sid>;
    using Proxy      = QueueingServiceProxy<MessageTrait>;
    std::shared_ptr<ClientComp> cls[NClient];
    int i = 0;
    for(auto& c : cls)
    {
        auto proxy = Proxy::createProxy(connectionType, addr, sid);
        c = std::make_shared<ClientComp>(proxy);
        c->setName("Client component " + std::to_string(i++));
        c->startTest();
    }

    maf::test::ServerComponent<MessageTrait, 0> server;
    server.setName("Server Component ");
    server.startTest(connectionType, addr, false);

    for(auto& c : cls)
    {
        c->stopTest();
    }
    maf::Logger::debug("End test");
}

#include <iostream>



int main()
{
    maf::Logger::init(maf::logging::LOG_LEVEL_DEBUG | maf::logging::LOG_LEVEL_FROM_INFO,
        [](const std::string& msg) {
            std::cout << msg << std::endl;
        },
        [](const std::string& msg) {
            std::cerr << msg << std::endl;
        });
    maf::util::TimeMeasurement tm([](auto t){
        maf::Logger::info("Total time is: ", static_cast<double>(t.count()) / 1000, "ms");
    });

    test<DefaultMessageTrait, 2>("app_internal", {});
    test<SerializableMessageTrait, 2>("local_ipc", Address{"com.opswat.client", 0});

    return 0;
}

