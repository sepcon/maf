#include "thaf/messaging/client-server/Address.h"
#include "thaf/messaging/client-server/CSStatus.h"
#include "thaf/utils/serialization/ByteArray.h"
#include "thaf/messaging/client-server/ipc/internal/platforms/windows/NamedPipeSender.h"
#include "thaf/messaging/client-server/ipc/internal/platforms/windows/PipeShared.h"
#include "thaf/utils/debugging/Debug.h"
#include <thread>
#include <atomic>
#include <windows.h>


namespace thaf {
namespace messaging {
namespace ipc {

constexpr int MAX_RETRY_TIMES = 5;

HANDLE NamedPipeSender::openPipe()
{
    return CreateFileA(
        _pipeName.c_str(),   // pipe name
        GENERIC_WRITE,  // write only
        0,              // no sharing
        nullptr,        // default security attributes
        OPEN_EXISTING,  // opens existing pipe
        0,              // default attributes
        nullptr);       // no template file
}

NamedPipeSender::~NamedPipeSender() {}


DataTransmissionErrorCode NamedPipeSender::send(const srz::ByteArray& ba)
{
    auto errCode = DataTransmissionErrorCode::ReceiverUnavailable;
    bool success = false;
    if(checkReceiverStatus() == Availability::Available)
    {
        int retryTimes = 0;
        while(retryTimes < MAX_RETRY_TIMES)
        {
            AutoCloseHandle pipeHandle = openPipe();
            if(pipeHandle != INVALID_HANDLE_VALUE)
            {
                DWORD dwMode = PIPE_READMODE_BYTE;
                success = SetNamedPipeHandleState(
                    pipeHandle,     // pipe handle
                    &dwMode,        // new pipe mode
                    nullptr,        // don't set maximum bytes
                    nullptr);       // don't set maximum time

                if (!success)
                {
                    thafErr("SetNamedPipeHandleState failed. LastError = " << GetLastError());
                    break;
                }

                // Send a message to the pipe server.
                DWORD writtenByteCount = 0;
                success = WriteFile(
                    pipeHandle,             // pipe handle
                    ba.firstpos(),             // message
                    static_cast<DWORD>(ba.length()),              // message length
                    &writtenByteCount,             // bytes written
                    nullptr);                  // not overlapped

                if (!success)
                {
                    thafErr("WriteFile to pipe failed. LastError = " << GetLastError());
                }
                else
                {
                    thafInfo("Bytes sent to address: " << _pipeName);
                }
                break;
            }

            else if(GetLastError() == ERROR_PIPE_BUSY)
            {
                thafErr("Receiver is being busy, then retry: " << ++retryTimes);
                std::this_thread::yield();
                continue;
            }
            else
            {
                break;
            }
        }

        if(success)
        {
            errCode = DataTransmissionErrorCode::Success;
        }
        else
        {
            if(GetLastError() == ERROR_PIPE_BUSY)
            {
                thafErr("Give up trying send byte to receiver!");
                errCode = DataTransmissionErrorCode::ReceiverBusy;
            }
            else
            {
                errCode = DataTransmissionErrorCode::ReceiverUnavailable;
            }
        }
    }
    else
    {
//        errCode = DataTransmissionErrorCode::ReceiverUnavailable;
        thafWarn("Receiver is not available for sending message, sender's address = " << _pipeName);
    }
    return errCode;
}


}}}
