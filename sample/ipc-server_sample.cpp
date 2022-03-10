#include <maf/LocalIPCStub.h>
#include <maf/utils/TimeMeasurement.h>

#include <ctime>
#include <future>
#include <iomanip>
#include <iostream>

#include "Server.h"

using namespace maf;

int main() {
  std::cout.sync_with_stdio(false);
  auto logFunc = [](const std::string &msg) {
    static std::mutex m;
    std::lock_guard lock(m);
    std::cout << msg << '\n';
  };
  auto errFunc = [](const std::string &msg) {
    std::cerr << "[ " << std::this_thread::get_id() << " ]" << msg << std::endl;
  };

  maf::logging::init(
      maf::logging::LOG_LEVEL_FROM_ERROR /*| maf::logging::LOG_LEVEL_DEBUG*/,
      std::move(logFunc), std::move(errFunc));

  std::vector<std::future<void>> serverStopSignals;

  MAF_LOGGER_DEBUG("[][][]Server is starting up!");
  localipc::StubPtr stub;
  int count = 0;
  do {
    stub = localipc::createStub({SERVER_NAME, WEATHER_SERVER_PORT},
                                SID_WeatherService);
    if (!stub) {
      MAF_LOGGER_ERROR("Failed to create stub for ", count, " times!");
      std::this_thread::sleep_for(2s);
    } else {
      break;
    }
  } while (true);

  ServerProcessor server(localipc::createStub(
      {SERVER_NAME, WEATHER_SERVER_PORT}, SID_WeatherService));

  for (int i = 0; i < 1; ++i) {
    serverStopSignals.emplace_back(
        std::async(std::launch::async, [&server] { server.run(); }));
  }

  for (auto &waiter : serverStopSignals) {
    if (waiter.valid()) {
      waiter.wait();
    }
  }
  for (auto &waiter : serverStopSignals) {
    if (waiter.valid()) {
      waiter.wait();
    }
  }
  maf::messaging::csmgmt::shutdownAllServers();
  MAF_LOGGER_DEBUG("[][][]Processor shutdown!");
}
