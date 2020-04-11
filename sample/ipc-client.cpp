#include <maf/messaging/client-server/ipc/LocalIPCProxy.h>
#include <maf/utils/TimeMeasurement.h>
#include "Client.h"
#include <iostream>

using namespace maf::messaging::ipc;
using namespace maf;

static const auto SERVER_ADDRESS = Address{SERVER_NAME, WEATHER_SERVER_PORT};

int main() {
  logging::init(
      logging::LOG_LEVEL_FROM_INFO | maf::logging::LOG_LEVEL_DEBUG,
      [](const std::string &msg) { std::cout << msg << std::endl; },
      [](const std::string &msg) { std::cerr << msg << std::endl; });

  util::TimeMeasurement tmeasure([](auto time) {
    MAF_LOGGER_DEBUG("Total execution time = ",
                     static_cast<double>(time.count()) / 1000, "ms");
  });

  MAF_LOGGER_DEBUG("Client is starting up!");

  ClientComponent{local::createProxy(SERVER_ADDRESS, SID_WeatherService)}
      .onMessage<EndOfRequestChainMsg>([](auto) { RunningComponent::stop(); })
      .run();

  MAF_LOGGER_DEBUG("Client shutdown!");

  return 0;
}
