#pragma once

#include <maf/messaging/client-server/Address.h>
#include <windows.h>

namespace maf {
namespace messaging {
namespace ipc {

static constexpr int WAIT_DURATION_MAX = 5000; // milliseconds
static constexpr int BUFFER_SIZE = 500;        // bytes

using ByteArrayPtr = std::shared_ptr<srz::ByteArray>;

inline std::string constructPipeName(const Address &pAddr) {
  return "\\\\.\\pipe\\ipc.messaging.maf\\" + pAddr.get_name() + ":" +
         std::to_string(pAddr.get_port());
}
struct AutoCloseHandle {
  AutoCloseHandle(HANDLE h = INVALID_HANDLE_VALUE) : _h(h) {}
  ~AutoCloseHandle() {
    if (_h != INVALID_HANDLE_VALUE) {
      CloseHandle(_h);
    }
  }
  operator HANDLE() { return _h; }
  HANDLE _h;
};

} // namespace ipc
} // namespace messaging
} // namespace maf
