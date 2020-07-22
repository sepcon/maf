#pragma once

#include <maf/messaging/ComponentDef.h>

namespace maf {
namespace messaging {
namespace global_threadpool {

using TaskType = Execution;

void init() noexcept;
void deinit() noexcept;
bool submit(TaskType task) noexcept;
bool tryAddThread() noexcept;
bool tryRemoveThread() noexcept;
size_t threadCount() noexcept;

}  // namespace global_threadpool
}  // namespace messaging
}  // namespace maf
