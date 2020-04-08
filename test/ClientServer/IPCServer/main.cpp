#include "../Server.h"
#include <maf/messaging/client-server/ipc/LocalIPCStub.h>
#include <maf/utils/TimeMeasurement.h>

using namespace maf::messaging::ipc;
using namespace maf::messaging;

#include <iostream>

int main() {
  std::cout.sync_with_stdio(false);
  auto logFunc = [](const std::string &msg) {
    std::cout << "[ " << std::this_thread::get_id() << " ]" << msg << std::endl;
  };
  auto errFunc = [](const std::string &msg) {
    std::cerr << "[ " << std::this_thread::get_id() << " ]" << msg << std::endl;
  };

  maf::logging::init(maf::logging::LOG_LEVEL_FROM_WARN |
                         maf::logging::LOG_LEVEL_DEBUG,
                     std::move(logFunc), std::move(errFunc));

  std::vector<std::future<void>> serverWaiters;

  MAF_LOGGER_DEBUG("Server is starting up!");
  ServerComponent server(local::createStub({SERVER_NAME, WEATHER_SERVER_PORT},
                                           SID_WeatherService));

  for (int i = 0; i < 5; ++i) {
    serverWaiters.emplace_back(server.runAsync());
  }
  for (auto &waiter : serverWaiters) {
    if (waiter.valid()) {
      waiter.wait();
    }
  }
  for (auto &waiter : serverWaiters) {
    if (waiter.valid()) {
      waiter.wait();
    }
  }

  MAF_LOGGER_DEBUG("Component shutdown!");
}