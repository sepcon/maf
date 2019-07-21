#pragma once

#include "thaf/messaging/client-server/interfaces/Address.h"
#include <windows.h>

namespace thaf {
namespace messaging {
namespace ipc {

static constexpr int WAIT_DURATION_MAX = 5000; // milliseconds
static constexpr int BUFFER_SIZE = 1000;       // bytes

inline std::string constructPipeName(const Address& pAddr)
{
    return "\\\\.\\pipe\\ipc.messaging.thaf\\" + pAddr.get_name() + ":" + std::to_string(pAddr.get_port());
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
} // thaf
