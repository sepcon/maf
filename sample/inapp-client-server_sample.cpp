#include <maf/ITCProxy.h>
#include <maf/ITCStub.h>
#include <maf/logging/Logger.h>
#include <maf/messaging/AsyncComponent.h>
#include <maf/utils/TimeMeasurement.h>

#include <iostream>
#include <string>

#include "Client.h"
#include "Server.h"

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

  auto proxy = itc::createProxy("helloworld");
  auto stub = itc::createStub("helloworld");

  ServerComponent<itc::Stub> server{stub};
  auto asyncServer = AsyncComponent{server.instance()};
  std::vector<std::unique_ptr<ClientComponent<itc::Proxy>>> clients;
  asyncServer.run();

  auto clientComponent = ClientComponent{ proxy };

  clientComponent.onMessage<EndOfRequestChainMsg>(
      [](auto) { this_component::stop(); });

  clientComponent.run();
  asyncServer.stopAndWait();

  return 0;
}
