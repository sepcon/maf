#include <iostream>
#include <maf/IThCProxy.h>
#include <maf/IThCStub.h>
#include <maf/logging/Logger.h>
#include <maf/utils/TimeMeasurement.h>

#include "Client.h"
#include "Server.h"
#include <string>

int main() {
  using namespace maf;
  maf::logging::init(
      maf::logging::LOG_LEVEL_DEBUG | maf::logging::LOG_LEVEL_FROM_INFO,
      [](const std::string &msg) { std::cout << msg << std::endl; },
      [](const std::string &msg) { std::cerr << msg << std::endl; });

  maf::util::TimeMeasurement tm([](auto t) {
    MAF_LOGGER_DEBUG("Total time is: ", static_cast<double>(t.count()) / 1000,
                     "ms");
  });

  auto proxy = ithc::createProxy("helloworld");
  auto stub = ithc::createStub("helloworld");

  ServerComponent<ithc::Stub> server{stub};
  std::vector<std::unique_ptr<ClientComponent<ithc::Proxy>>> clients;
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
