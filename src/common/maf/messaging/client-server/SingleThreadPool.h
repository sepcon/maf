#pragma once

#include <functional>

namespace maf {
namespace messaging {
namespace single_threadpool {

using TaskType = std::function<void()>;

void init();
void deinit();
bool submit(TaskType task);

}  // namespace single_threadpool
}  // namespace messaging
}  // namespace maf
