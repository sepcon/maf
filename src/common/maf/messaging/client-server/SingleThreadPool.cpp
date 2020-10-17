#include "SingleThreadPool.h"

#include <maf/messaging/ComponentEx.h>

namespace maf {
namespace messaging {
namespace single_threadpool {

class ThreadPool : public ComponentExBase {
  std::thread thread_;

 public:
  void launch() {
    thread_ = std::thread{[this] { instance_->run(); }};
  }

  void stopAndWait() {
    instance()->stop();
    if (thread_.joinable()) {
      thread_.join();
    }
  }
};
static ThreadPool& thepool() {
  static ThreadPool _;
  return _;
}

bool submit(TaskType task) { return thepool()->execute(std::move(task)); }
void init() { thepool().launch(); }
void deinit() { thepool().stopAndWait(); }

}  // namespace single_threadpool
}  // namespace messaging
}  // namespace maf
