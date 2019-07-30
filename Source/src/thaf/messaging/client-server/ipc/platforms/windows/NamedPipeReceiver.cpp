#include "thaf/messaging/client-server/ipc/internal/platforms/windows/LocalIPCSender.h"
#include "thaf/messaging/client-server/ipc/internal/platforms/windows/NamedPipeReceiver.h"
#include "thaf/messaging/client-server/ipc/internal/platforms/windows/PipeShared.h"
#include "thaf/utils/serialization/ByteArray.h"
#include "thaf/utils/debugging/Debug.h"
#include <thread>
#include <windows.h>
#include <stdio.h>


namespace thaf {
namespace messaging {
namespace ipc {


void NamedPipeReceiver::listningThreadFunction() {
    bool bConnected = false;
    while (!_stopped.load(std::memory_order_acquire))
    {
        _pipeHandle = CreateNamedPipeA(
            _pipeName.c_str(),          // pipe name
            PIPE_ACCESS_INBOUND,        // read/write access
            PIPE_TYPE_BYTE |            // byte type pipe
                PIPE_READMODE_BYTE |        // byte-read mode
                PIPE_WAIT,                  // blocking mode
            PIPE_UNLIMITED_INSTANCES,   // max. instances
            0,                          // output buffer size
            BUFFER_SIZE,                    // input buffer size
            0,                          // client time-out
            nullptr);                   // default security attribute

        if (_pipeHandle == INVALID_HANDLE_VALUE)
        {
            thafErr("CreateNamedPipe failed, LastError = " << GetLastError());
            break;
        }

        bConnected = ConnectNamedPipe(_pipeHandle, nullptr) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
        if(_stopped.load(std::memory_order_acquire)) { break; }

        if (bConnected)
        {
            DWORD cbBytesRead = 0;
            bool fSuccess = false;
            auto pBytes = std::make_shared<srz::ByteArray>();
            pBytes->resize(BUFFER_SIZE);

            fSuccess = ReadFile( _pipeHandle,        // handle to pipe
                                pBytes->firstpos(), // buffer to receive data
                                BUFFER_SIZE,            // size of buffer
                                &cbBytesRead,       // number of bytes read
                                nullptr);           // not overlapped I/O

            if (!fSuccess || cbBytesRead == 0)
            {
                if (GetLastError() == ERROR_BROKEN_PIPE)
                {
                    thafErr("Broken pipe: " << _pipeName);
                }
                else
                {
                    thafErr("ReadFile failed, LastError = " << GetLastError());
                }
            }
            else
            {
                pBytes->resize(cbBytesRead);
                notifyObervers(pBytes);
            }
        }
        else
        {
            // The client could not connect, so close the pipe.
            CloseHandle(_pipeHandle);
            std::this_thread::yield();
        }
    }
    _stopped.store(true, std::memory_order_release);
    thafInfo("NamedPipeReceiver thread[" << std::this_thread::get_id() << "] stops listening!" );
}

bool NamedPipeReceiver::stopListening()
{
    _stopped.store(true, std::memory_order_release);
    LocalIPCSender closeRequester;
    closeRequester.initConnection(_myaddr);
    closeRequester.send("stop");
    waitForWorkerThreadToStop();
    return true;
}




} // ipc
} // messaging
} // thaf
