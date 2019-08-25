#pragma once

#include "maf/messaging/client-server/Address.h"
#include <windows.h>

namespace maf {
namespace messaging {
namespace ipc {

static constexpr int WAIT_DURATION_MAX = 5000; // milliseconds
static constexpr int BUFFER_SIZE = 500;       // bytes

inline std::string constructPipeName(const Address& pAddr)
{
    return "\\\\.\\pipe\\ipc.messaging.maf\\" + pAddr.name() + ":" + std::to_string(pAddr.port());
}
struct AutoCloseHandle
{
    AutoCloseHandle(HANDLE h = INVALID_HANDLE_VALUE) : _h(h) {}
    ~AutoCloseHandle()
    {
        if (_h != INVALID_HANDLE_VALUE)
        {
            CloseHandle(_h);
        }
    }
    operator HANDLE()
    {
        return _h;
    }
    HANDLE _h;
};

} // ipc
} // messaging
} // maf
