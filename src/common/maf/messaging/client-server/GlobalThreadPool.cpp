#include "GlobalThreadPool.h"

#include <maf/logging/Logger.h>
#include <maf/messaging/Component.h>
#include <maf/threading/Lockable.h>

#include <thread>
#include <vector>

namespace maf {
namespace messaging {
namespace global_threadpool {

struct ThreadInterupt {};

struct ThePool {
  ThePool() {}

  void init() {
    details = Component::create();
  }

  auto threadCount() { return threads.atomic()->size(); }

  void deinit() {
    details->stop();
    auto atThreads = threads.atomic();
    for (auto& th : *atThreads) {
      if (th.joinable()) {
        th.join();
      }
    }
    atThreads->clear();
  }

  bool tryAddThread() {
    try {
      threads.atomic()->emplace_back([this] {
        try {
          this->details->run();
        } catch (const ThreadInterupt&) {
          MAF_LOGGER_INFO("Thread id ", std::this_thread::get_id(), " stopped");
        }
      });
      return true;
    } catch (const std::exception& e) {
      MAF_LOGGER_ERROR("Failed to add new thread to global thread pool: ",
                       e.what());
    }
    return false;
  }

  bool tryRemoveThread() {
    return details->execute([] { throw ThreadInterupt{}; });
  }

  ComponentInstance details;
  threading::Lockable<std::vector<std::thread>> threads;
};
static ThePool& thepool() {
  static ThePool _;
  return _;
}

bool submit(TaskType task) noexcept {
  return thepool().details->execute(std::move(task));
}

bool tryAddThread() noexcept { return thepool().tryAddThread(); }

void init() noexcept { thepool().init(); }

void deinit() noexcept { thepool().deinit(); }

bool tryRemoveThread() noexcept { return thepool().tryRemoveThread(); }

size_t threadCount() noexcept { return thepool().threadCount(); }

}  // namespace global_threadpool
}  // namespace messaging
}  // namespace maf
