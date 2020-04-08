#pragma once

#include "IPCClientBase.h"

namespace maf {
namespace messaging {
namespace ipc {

class LocalIPCClient : public IPCClientBase {
public:
  LocalIPCClient() : IPCClientBase{IPCType::Local} {}
};

} // namespace ipc
} // namespace messaging
} // namespace maf
