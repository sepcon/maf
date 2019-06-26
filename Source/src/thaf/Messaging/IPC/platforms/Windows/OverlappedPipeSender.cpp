#include "thaf/Messaging/IPC/Prv/Platforms/Windows/OverlappedPipeSender.h"
#include "thaf/Messaging/IPC/Prv/Platforms/Windows/PipeShared.h"
#include "thaf/Utils/Debugging/Debug.h"
#include <thread>
#include <atomic>

namespace thaf {
namespace messaging {
namespace ipc {

static constexpr int MAX_ATEMPTS = 10;

OverlappedPipeSender::OverlappedPipeSender()
{

}

OverlappedPipeSender::~OverlappedPipeSender()
{

}

ConnectionErrorCode OverlappedPipeSender::send(const srz::ByteArray &ba)
{
    auto errCode = ConnectionErrorCode::Failed;
    bool success = false;
    if(checkServerStatus() == ConnectionStatus::Available)
    {
        int retryTimes = 0;
        while(retryTimes < MAX_ATEMPTS)
        {
            memset(&_oOverlap, 0, sizeof(_oOverlap));
			AutoCloseHandle pipeHandle = openPipe();
            bool shouldRetry = false;
            if(pipeHandle != INVALID_HANDLE_VALUE)
            {
                AutoCloseHandle hEvent = CreateEvent(
                    nullptr,    // default security attribute
                    TRUE,       // manual-reset event
                    TRUE,       // initial state = signaled
                    nullptr);

                _oOverlap.hEvent = hEvent;

                // Send a message to the pipe server.
                success = WriteFile(
                    pipeHandle,                                     // pipe handle
                    ba.firstpos(),                                  // message
                    static_cast<DWORD>(ba.length()),                // message length
                    nullptr,                                        // bytes written
                    &_oOverlap);                                    // overlapped

                if (!success)
                {
                    if(GetLastError() == ERROR_IO_PENDING)
                    {
                        DWORD writtenByteCount = 0;
                        auto waitIdx = WaitForSingleObject(_oOverlap.hEvent, 4000);
                        if(waitIdx != WAIT_OBJECT_0)
                        {
                            CancelIo(pipeHandle);
                            GetOverlappedResult(pipeHandle, &_oOverlap, &writtenByteCount, TRUE);
                            thafErr("Error while waiting for bytes to be transffered to receiver");
                        }
                        else
                        {
                            success = GetOverlappedResult(pipeHandle, &_oOverlap, &writtenByteCount, TRUE);
                            if(success && (writtenByteCount == ba.length()))
                            {
                                thafInfo("Sent " << ba.length() << " bytes to receiver successful!");
                            }
                            else
                            {
                                thafErr("Could not transfer completely " << ba.length() << " bytes to receiver!");
                            }
                        }
                    }
                    else
                    {
                        thafErr("sending bytes failed with error: " << GetLastError());
                    }
                }
                else
                {
                    thafInfo(ba.size() << " bytes sent to address: " << _pipeName);
                }
            }
            else if(GetLastError() == ERROR_PIPE_BUSY)
            {
                shouldRetry = true;
                thafErr("Receiver is being busy, then retry: " << ++retryTimes);
            }
			else
			{
				thafErr("Connect pipe with error: " << GetLastError());
			}

            if (success || !shouldRetry)
			{
                FlushFileBuffers(pipeHandle);
				break;
            }
            else
			{
                auto randomTime = std::rand() % 100;
                std::this_thread::sleep_for(std::chrono::milliseconds(randomTime));
                thafWarn("Retry to send " << ba.size() << " bytes to server " << _pipeName);
			}
        }

        if(success)
        {
            errCode = ConnectionErrorCode::Success;
        }
        else if(GetLastError() == ERROR_PIPE_BUSY)
        {
            errCode = ConnectionErrorCode::Busy;
            if(retryTimes >= MAX_ATEMPTS)
            {
                thafErr("Give up trying send byte to receiver!");
            }
        }
        else
        {
            errCode = ConnectionErrorCode::Failed;
        }
    }
    else
    {
//        errCode = ConnectionErrorCode::Failed; //dont need to set here, it must be default failed
        thafWarn("Server is not available for sending message, sender's address = " << _pipeName);
    }

    return errCode;
}

HANDLE OverlappedPipeSender::openPipe()
{
    return CreateFileA(
        _pipeName.c_str(),                          // pipe name
        GENERIC_WRITE |                             // write only
        FILE_FLAG_OVERLAPPED,
        0,                                          // no sharing
        nullptr,                                    // default security attributes
        OPEN_EXISTING,                              // opens existing pipe
        0,                                          // write overlapped
        nullptr);                                   // no template file
}


} // ipc
} // messaging
} // thaf
