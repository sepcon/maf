#pragma once

#include "IPCServerBase.h"
#include <maf/patterns/Patterns.h>

namespace maf {
namespace messaging {
namespace ipc {

class LocalIPCServer : public IPCServerBase, pattern::Unasignable {
public:
  LocalIPCServer() : IPCServerBase(IPCType::Local) {}
};

} // namespace ipc
} // namespace messaging
} // namespace maf
