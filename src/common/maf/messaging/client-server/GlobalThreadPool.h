#pragma once

#include <maf/messaging/ComponentDef.h>

namespace maf {
namespace messaging {
namespace global_threadpool {

using TaskType = Execution;

void init();
void deinit();
bool submit(TaskType task);
bool tryAddThread();
bool tryRemoveThread();
size_t threadCount();

}  // namespace global_threadpool
}  // namespace messaging
}  // namespace maf
