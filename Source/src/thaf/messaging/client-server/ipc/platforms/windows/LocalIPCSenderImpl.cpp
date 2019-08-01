#include "thaf/messaging/client-server/ipc/internal/platforms/windows/LocalIPCSenderImpl.h"
#include "thaf/messaging/client-server/ipc/internal/platforms/windows/PipeShared.h"
#include "thaf/utils/TimeMeasurement.h"
#include "thaf/utils/debugging/Debug.h"
#include <thread>
#include <atomic>

namespace thaf {
namespace messaging {
namespace ipc {

static constexpr int MAX_ATEMPTS = 10;

LocalIPCSenderImpl::LocalIPCSenderImpl()
{

}

LocalIPCSenderImpl::~LocalIPCSenderImpl()
{

}

DataTransmissionErrorCode LocalIPCSenderImpl::send(const srz::ByteArray &ba)
{
    auto errCode = DataTransmissionErrorCode::ReceiverUnavailable;
    bool success = false;
    if(checkReceiverStatus() == Availability::Available)
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
            }
			else
			{
				thafErr("Connect pipe with error: " << GetLastError());
			}

            if (success || !shouldRetry)
			{
//                FlushFileBuffers(pipeHandle);
				break;
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 100));
                thafWarn("Retry to send " << ba.size() << " bytes " << ++retryTimes << " times to server " << _pipeName);
			}
        }

        if(success)
        {
            errCode = DataTransmissionErrorCode::Success;
        }
        else if(GetLastError() == ERROR_PIPE_BUSY)
        {
            errCode = DataTransmissionErrorCode::ReceiverBusy;
            if(retryTimes >= MAX_ATEMPTS)
            {
                thafErr("Give up trying send byte to receiver!");
            }
        }
        else
        {
            errCode = DataTransmissionErrorCode::ReceiverUnavailable;
        }
    }
    else
    {
//        errCode = DataTransmissionErrorCode::ReceiverUnavailable; //dont need to set here, it must be default failed
        thafWarn("Receiver is not available for receiving message, receiver's address = " << _pipeName);
    }

    return errCode;
}

HANDLE LocalIPCSenderImpl::openPipe()
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
