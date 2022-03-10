#include <maf/LocalIPCProxy.h>
#include <maf/utils/TimeMeasurement.h>

#include <iomanip>
#include <iostream>

#include "Client.h"

using namespace maf;

static const auto DataTransmissionServerAddress =
    Address{SERVER_NAME, WEATHER_SERVER_PORT};
struct LogRequest {
  std::string msg;
};

int main() {
  std::cout.sync_with_stdio(false);
  auto mainComp = Processor::create("main");

  mainComp->connect<LogRequest>(
      [](LogRequest msg) { std::cout << msg.msg << "\n"; });

  logging::init(
      logging::LOG_LEVEL_FROM_ERROR | maf::logging::LOG_LEVEL_DEBUG |
          maf::logging::LOG_LEVEL_VERBOSE,
      [mainComp](const std::string &msg) { mainComp->post(LogRequest{msg}); },
      [](const std::string &msg) { std::cerr << msg << std::endl; });

  util::TimeMeasurement tmeasure([](auto time) {
    std::cout << "Total execution time = "
              << static_cast<double>(time.count()) / 1000 << "ms" << std::endl;
  });

  MAF_LOGGER_DEBUG("[][][]Client is starting up!");

  auto clientProcessor = new ClientProcessor{
      localipc::createProxy(DataTransmissionServerAddress, SID_WeatherService)};

  (*clientProcessor)->connect<EndOfRequestChainMsg>([mainComp](auto) {
    mainComp->stop();
    this_processor::stop();
  });

  std::thread clientThread{[&] { clientProcessor->run(); }};

  mainComp->run();
  clientThread.join();
  delete clientProcessor;

  csmgmt::shutdownAllClients();
  MAF_LOGGER_DEBUG("[][][]Client shutdown!");

  return 0;
}
