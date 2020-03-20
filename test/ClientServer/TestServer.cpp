#include "ControllableInterface.h"
#include "Server.h"
#include <maf/export/MafExport_global.h>
#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/DefaultMessageTrait.h>

namespace maf {
namespace test {
using namespace messaging;

class TestServer : public ControllableDefault
{
  using MessageTrait = messaging::DefaultMessageTrait;
  using ClientComp = maf::test::ServerComponent<MessageTrait>;

public:
  TestServer();
  void init() override;
  void deinit() override;
  void start() override;
  maf::test::ServerComponent<MessageTrait> server;
};

TestServer::TestServer()
{
  MAF_LOGGER_DEBUG("Test server created!");
}

void
TestServer::init()
{
  MAF_LOGGER_DEBUG("Test server initializing....");

  server.setName("Server Component ");
  MAF_LOGGER_DEBUG("Test server initialized successfully");
}

void
TestServer::deinit()
{
  server.stopTest();
  MAF_LOGGER_DEBUG("Test server deinitialized successfully");
}

void
TestServer::start()
{
  server.startTest("app_internal", { "com.github.sepcon", 0 });
}

} // test
} // maf

MAF_PLUGIN(maf::test::TestServer, "TestServer", "1");
