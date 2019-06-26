#pragma once

#include "thaf/Messaging/IPC/Address.h"
#include <windows.h>

namespace thaf {
namespace messaging {
namespace ipc {

inline std::string constructPipeName(const Address& pAddr)
{
    return pAddr.get_name() + ":" + std::to_string(pAddr.get_port());
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
