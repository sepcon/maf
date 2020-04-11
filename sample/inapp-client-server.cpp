#include <iostream>
#include <maf/logging/Logger.h>
#include <maf/messaging/ExtensibleComponent.h>
#include <maf/messaging/client-server/IAProxy.h>
#include <maf/messaging/client-server/IAStub.h>
#include <maf/utils/TimeMeasurement.h>

#include "Client.h"
#include "Server.h"
#include <string>

int main() {
  using namespace maf::messaging;
  maf::logging::init(
      maf::logging::LOG_LEVEL_DEBUG | maf::logging::LOG_LEVEL_FROM_INFO,
      [](const std::string &msg) { std::cout << msg << std::endl; },
      [](const std::string &msg) { std::cerr << msg << std::endl; });

  maf::util::TimeMeasurement tm([](auto t) {
    MAF_LOGGER_DEBUG("Total time is: ", static_cast<double>(t.count()) / 1000,
                     "ms");
  });

  auto proxy = inapp::createProxy("helloworld");
  auto stub = inapp::createStub("helloworld");

  ServerComponent<inapp::Stub> server{stub};
  std::vector<std::unique_ptr<ClientComponent<inapp::Proxy>>> clients;
  auto serverFuture = server.runAsync();
  auto client = ClientComponent{proxy};
  client
      .onMessage<EndOfRequestChainMsg>([&server](auto) {
        RunningComponent::stop();
        server.stop();
      })
      .run();
  tm.stop();
  return 0;
}
