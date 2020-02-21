#include "TestClient.h"
#include "Client.h"
#include "ControllableInterface.h"
#include <maf/export/MafExport_global.h>
#include <maf/messaging/Component.h>
#include <maf/messaging/client-server/DefaultMessageTrait.h>
#include <maf/logging/Logger.h>
#include "WeatherContract.h"

namespace maf {
namespace test {
using namespace messaging;

class TestClient : public ControllableDefault
{
    using MessageTrait = DefaultMessageTrait;
    using ClientComp = maf::test::ClientComponent<MessageTrait>;
    using Proxy      = QueueingServiceProxy<MessageTrait>;
public:
    TestClient();
    void init() override;
    void deinit() override;
    void start() override;
    std::shared_ptr<ClientComp> cls[1];
};


TestClient::TestClient()
{
    logging::Logger::debug("Test client created!");
}

void TestClient::init()
{
    logging::Logger::debug("Test client initializing....");
    if(auto component = Component::getActiveSharedPtr())
    {
        Logger::debug("My component is: ", component->name());
    }
    else
    {
        Logger::debug("No active component!");
    }

    int i = 0;
    for(auto& c : cls)
    {
        auto proxy = Proxy::createProxy(
            "app_internal",
            {"com.github.sepcon", 0},
            "weather_service"
            );

        c = std::make_shared<ClientComp>(proxy);
        c->setName("Client component " + std::to_string(i++));
    }
    logging::Logger::debug("Test client initialized successfully");
}

void TestClient::deinit()
{
    for(auto& c : cls)
    {
        c->stopTest();
    }
    logging::Logger::debug("Test client deinitialized successfully");
}

void TestClient::start()
{
    ConnectionType contype = "app_internal";
    Address serverAddr = {SERVER_ADDRESS, WEATHER_SERVER_PORT};
    for(auto& c : cls)
    {
        logging::Logger::debug(
            "Client compoent ", c->name(), " starts!"
            );

        c->startTest(contype, serverAddr);
    }
}


} //test
} //maf

MAF_PLUGIN(maf::test::TestClient, "TestClient", "1");


