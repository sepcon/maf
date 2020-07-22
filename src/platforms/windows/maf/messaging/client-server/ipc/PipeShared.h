#pragma once

#include <maf/messaging/client-server/Address.h>
#include <maf/utils/serialization/Buffer.h>
#include <windows.h>

namespace maf {
namespace messaging {
namespace ipc {

inline constexpr int WAIT_DURATION_MAX = 5000;  // milliseconds
inline constexpr int BUFFER_SIZE = 1000;         // bytes

using ByteArrayPtr = std::shared_ptr<srz::Buffer>;
using PipeNameType = std::string;

inline PipeNameType constructPipeName(const Address &pAddr) {
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

}  // namespace ipc
}  // namespace messaging
}  // namespace maf
