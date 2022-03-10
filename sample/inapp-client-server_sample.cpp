#include <maf/ITCProxy.h>
#include <maf/ITCStub.h>
#include <maf/logging/Logger.h>
#include <maf/messaging/ProcessorEx.h>
#include <maf/utils/TimeMeasurement.h>

#include <iostream>
#include <string>

#include "Client.h"
#include "Server.h"

int main() {
  using namespace maf;
  maf::logging::init(
      maf::logging::LOG_LEVEL_DEBUG | maf::logging::LOG_LEVEL_VERBOSE,
      [](const std::string &msg) { std::cout << msg << std::endl; },
      [](const std::string &msg) { std::cerr << msg << std::endl; });

  maf::util::TimeMeasurement tm([](auto t) {
    MAF_LOGGER_DEBUG("Total time is: ", static_cast<double>(t.count()) / 1000,
                     "ms");
  });

  auto proxy = itc::createProxy("helloworld");
  auto stub = itc::createStub("helloworld");

  ServerProcessor<itc::Stub> server{stub};
  auto asyncServer = AsyncProcessor{server.instance()};
  std::vector<std::unique_ptr<ClientProcessor<itc::Proxy>>> clients;
  asyncServer.launch();

  auto clientProcessor = ClientProcessor{proxy};

  clientProcessor->connect<EndOfRequestChainMsg>(
      [](auto) { this_processor::stop(); });

  clientProcessor.run();
  asyncServer.stopAndWait();

  messaging::csmgmt::shutdownAllClients();
  messaging::csmgmt::shutdownAllServers();

  return 0;
}
