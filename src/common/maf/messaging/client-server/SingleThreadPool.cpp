#include "SingleThreadPool.h"

#include <maf/messaging/ProcessorEx.h>

namespace maf {
namespace messaging {
namespace single_threadpool {

class ThreadPool : public ProcessorExBase {
  std::thread thread_;

 public:
  ThreadPool() {
    std::thread{[this] { instance_->run(); }}.detach();
  }
};

static ThreadPool& instance() {
  static ThreadPool* _ = new ThreadPool;
  return *_;
}

bool submit(TaskType task) { return instance()->executeAsync(std::move(task)); }

}  // namespace single_threadpool
}  // namespace messaging
}  // namespace maf
