#include "TestClient.h"
#include "Client.h"
#include "ControllableInterface.h"
#include "WeatherContract.h"
#include <maf/export/MafExport_global.h>
#include <maf/logging/Logger.h>
#include <maf/messaging/Component.h>
#include <maf/messaging/client-server/DefaultMessageTrait.h>

namespace maf {
namespace test {
using namespace messaging;

class TestClient : public ControllableDefault
{
  using MessageTrait = DefaultMessageTrait;
  using ClientComp = maf::test::ClientComponent<MessageTrait>;

public:
  TestClient();
  void init() override;
  void deinit() override;
  void start() override;
  std::shared_ptr<ClientComp> cls[1];
};

TestClient::TestClient()
{
  MAF_LOGGER_DEBUG("Test client created!");
}

void
TestClient::init()
{
  MAF_LOGGER_DEBUG("Test client initializing....");
  if (auto component = RunningComponent::shared()) {
    MAF_LOGGER_DEBUG("My component is: ", component->name());
  } else {
    MAF_LOGGER_DEBUG("No active component!");
  }

  int i = 0;
  for (auto& c : cls) {
    auto proxy = Proxy<MessageTrait>::createProxy(
      "app_internal", { "com.github.sepcon", 0 }, "weather_service");

    c = std::make_shared<ClientComp>(proxy);
    c->setName("Client component " + std::to_string(i++));
  }
  MAF_LOGGER_DEBUG("Test client initialized successfully");
}

void
TestClient::deinit()
{
  for (auto& c : cls) {
    c->stopTest();
  }
  MAF_LOGGER_DEBUG("Test client deinitialized successfully");
}

void
TestClient::start()
{
  ConnectionType contype = "app_internal";
  Address serverAddr = { SERVER_ADDRESS, WEATHER_SERVER_PORT };
  for (auto& c : cls) {
    MAF_LOGGER_DEBUG("Client compoent ", c->name(), " starts!");

    c->startTest(contype, serverAddr);
  }
}

} // test
} // maf

MAF_PLUGIN(maf::test::TestClient, "TestClient", "1");
