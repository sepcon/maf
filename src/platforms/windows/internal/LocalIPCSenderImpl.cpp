#include "LocalIPCSenderImpl.h"
#include "PipeShared.h"
#include <maf/utils/debugging/Debug.h>
#include <thread>
#include <atomic>

namespace maf {
namespace messaging {
namespace ipc {

static constexpr int MAX_ATEMPTS = 10;
static HANDLE openPipe(const std::string& pipeName);
static bool writeToPipe(HANDLE pipeHandle, OVERLAPPED& overlapStructure, const char* buffer, size_t buffSize)
{
    bool success = false;
    if(pipeHandle != INVALID_HANDLE_VALUE)
    {
        AutoCloseHandle hEvent = CreateEvent(
            nullptr,    // default security attribute
            TRUE,       // manual-reset event
            TRUE,       // initial state = signaled
            nullptr);

        overlapStructure.hEvent = hEvent;

        // Send a message to the pipe server.
        success = WriteFile(
            pipeHandle,                                     // pipe handle
            buffer,                                         // message
            static_cast<DWORD>(buffSize),                   // message length
            nullptr,                                        // bytes written
            &overlapStructure);                                    // overlapped

        if (!success)
        {
            if(GetLastError() == ERROR_IO_PENDING)
            {
                DWORD writtenByteCount = 0;
                auto waitIdx = WaitForSingleObject(overlapStructure.hEvent, 4000);
                if(waitIdx != WAIT_OBJECT_0)
                {
                    CancelIo(pipeHandle);
                    GetOverlappedResult(pipeHandle, &overlapStructure, &writtenByteCount, TRUE);
                    mafErr("Error while waiting for bytes to be transffered to receiver");
                }
                else
                {
                    success = GetOverlappedResult(pipeHandle, &overlapStructure, &writtenByteCount, TRUE);
                    if(success && (writtenByteCount == buffSize))
                    {
                        mafInfo("Sent " << buffSize << " bytes to receiver successful!");
                    }
                    else
                    {
                        mafErr("Could not transfer completely " << buffSize << " bytes to receiver!");
                    }
                }
            }
            else
            {
                mafErr("sending bytes failed with error: " << GetLastError());
            }
        }
    }
    return success;
}

LocalIPCSenderImpl::LocalIPCSenderImpl()
{

}

LocalIPCSenderImpl::~LocalIPCSenderImpl()
{

}

DataTransmissionErrorCode LocalIPCSenderImpl::send(const srz::ByteArray &ba, const Address &destination)
{
    auto errCode = DataTransmissionErrorCode::ReceiverUnavailable;
    bool success = false;
    if(checkReceiverStatus() == Availability::Available)
    {
        int retryTimes = 0;
        while(retryTimes < MAX_ATEMPTS)
        {
            memset(&_oOverlap, 0, sizeof(_oOverlap));
            AutoCloseHandle pipeHandle = openPipe(destination.valid() ? constructPipeName(destination) : _pipeName);
            bool shouldRetry = false;
            if(pipeHandle != INVALID_HANDLE_VALUE)
            {
                uint32_t baSize = static_cast<uint32_t>(ba.size());
                if((success = writeToPipe(pipeHandle, _oOverlap, reinterpret_cast<const char*>(&baSize), sizeof(baSize))))
                {
                    success = writeToPipe(pipeHandle, _oOverlap, ba.firstpos(), ba.size());
                }
            }
            else if(GetLastError() == ERROR_PIPE_BUSY)
            {
                shouldRetry = true;
            }
            else
            {
                mafErr("Connect pipe with error: " << GetLastError());
            }

            if (success || !shouldRetry)
            {
                FlushFileBuffers(pipeHandle);
                break;
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 100));
                mafWarn("Retry to send " << ba.size() << " bytes " << ++retryTimes << " times to address " << _pipeName);
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
                mafErr("Give up trying send byte to receiver!");
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
        mafWarn("Receiver is not available for receiving message, receiver's address = " << _pipeName);
    }

    return errCode;
}

HANDLE openPipe(const std::string &pipeName)
{
    return CreateFileA(
                pipeName.c_str(),                                                                   // pipe name
                GENERIC_WRITE |                                                                     // write only
                FILE_FLAG_OVERLAPPED,
                0,                                                                                  // no sharing
                nullptr,                                                                            // default security attributes
                OPEN_EXISTING,                                                                      // opens existing pipe
                0,                                                                                  // write overlapped
                nullptr);                                                                           // no template file
}


} // ipc
} // messaging
} // maf
