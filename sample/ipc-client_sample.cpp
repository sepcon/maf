#include <maf/LocalIPCProxy.h>
#include <maf/utils/TimeMeasurement.h>

#include <iostream>

#include "Client.h"

using namespace maf;

static const auto DataTransmissionServerAddress =
    Address{SERVER_NAME, WEATHER_SERVER_PORT};

int main() {
  std::cout.sync_with_stdio(false);
  logging::init(
      logging::LOG_LEVEL_FROM_INFO | maf::logging::LOG_LEVEL_DEBUG /*|
                                     maf::logging::LOG_LEVEL_VERBOSE*/
      ,
      [](const std::string &msg) { std::cout << msg << std::endl; },
      [](const std::string &msg) { std::cerr << msg << std::endl; });

  util::TimeMeasurement tmeasure([](auto time) {
    std::cout << "Total execution time = "
              << static_cast<double>(time.count()) / 1000 << "ms" << std::endl;
  });

  MAF_LOGGER_DEBUG("Client is starting up!");

  auto clientComponent = ClientComponent{
      localipc::createProxy(DataTransmissionServerAddress, SID_WeatherService)};

  clientComponent->connect<EndOfRequestChainMsg>(
      [](auto) { this_component::stop(); });

  clientComponent.run();

  MAF_LOGGER_DEBUG("Client shutdown!");

  return 0;
}
