#include "LocalIPCSenderImpl.h"
#include "PipeShared.h"
#include <atomic>
#include <maf/logging/Logger.h>
#include <thread>

namespace maf {

namespace messaging {
namespace ipc {

static constexpr int MAX_ATEMPTS = 10;

static HANDLE openPipe(const std::string &pipeName);
static bool writeToPipe(HANDLE pipeHandle, OVERLAPPED &overlapStructure,
                        const char *buffer, size_t buffSize);

LocalIPCSenderImpl::LocalIPCSenderImpl() {}

LocalIPCSenderImpl::~LocalIPCSenderImpl() {}

ActionCallStatus LocalIPCSenderImpl::send(const srz::ByteArray &ba,
                                          const Address &destination) {
  auto errCode = ActionCallStatus::ReceiverUnavailable;
  bool success = false;
  if (checkReceiverStatus() == Availability::Available) {
    int retryTimes = 0;
    while (retryTimes < MAX_ATEMPTS) {
      memset(&oOverlap_, 0, sizeof(oOverlap_));
      AutoCloseHandle pipeHandle = openPipe(
          destination.valid() ? constructPipeName(destination) : pipeName_);
      if (pipeHandle != INVALID_HANDLE_VALUE) {
        uint32_t baSize = static_cast<uint32_t>(ba.size());
        if ((success = writeToPipe(pipeHandle, oOverlap_,
                                   reinterpret_cast<const char *>(&baSize),
                                   sizeof(baSize)))) {
          if ((success = writeToPipe(pipeHandle, oOverlap_, ba.data(),
                                     ba.size()))) {
            FlushFileBuffers(pipeHandle);
            break;
          }
        }
      } else if (GetLastError() == ERROR_PIPE_BUSY) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(std::rand() % 100));
        MAF_LOGGER_WARN(
            "Retry to send ", ba.size(), " bytes ", ++retryTimes,
            " times to address ",
            (destination.valid() ? destination : receiverAddress_).dump());
      } else {
        MAF_LOGGER_ERROR("Connect pipe with error: ", GetLastError());
        break;
      }
    }

    if (success) {
      errCode = ActionCallStatus::Success;
    } else if (GetLastError() == ERROR_PIPE_BUSY) {
      errCode = ActionCallStatus::ReceiverBusy;
      if (retryTimes >= MAX_ATEMPTS) {
        MAF_LOGGER_ERROR("Give up trying send byte to receiver!");
      }
    } else {
      errCode = ActionCallStatus::ReceiverUnavailable;
    }
  } else {
    // errCode = ActionCallStatus::ReceiverUnavailable;
    // dont need to set here, it must be default failed
    MAF_LOGGER_WARN(
        "Receiver is not available for receiving message, "
        "receiver's address = ",
        (destination.valid() ? destination : receiverAddress_).dump());
  }

  return errCode;
}

static HANDLE openPipe(const std::string &pipeName) {
  return CreateFileA(pipeName.c_str(), // pipe name
                     GENERIC_WRITE |   // write only
                         FILE_FLAG_OVERLAPPED,
                     0,             // no sharing
                     nullptr,       // default security attributes
                     OPEN_EXISTING, // opens existing pipe
                     0,             // write overlapped
                     nullptr);      // no template file
}

static bool writeToPipe(HANDLE pipeHandle, OVERLAPPED &overlapStructure,
                        const char *buffer, size_t buffSize) {
  bool success = false;
  if (pipeHandle != INVALID_HANDLE_VALUE) {
    AutoCloseHandle hEvent = CreateEvent(nullptr, // default security attribute
                                         TRUE,    // manual-reset event
                                         TRUE,    // initial state = signaled
                                         nullptr);

    overlapStructure.hEvent = hEvent;

    // Send a message to the pipe server.
    success = WriteFile(pipeHandle,                   // pipe handle
                        buffer,                       // message
                        static_cast<DWORD>(buffSize), // message length
                        nullptr,                      // bytes written
                        &overlapStructure);           // overlapped

    if (!success) {
      if (GetLastError() == ERROR_IO_PENDING) {
        DWORD writtenByteCount = 0;
        auto waitIdx = WaitForSingleObject(overlapStructure.hEvent, 4000);
        if (waitIdx != WAIT_OBJECT_0) {
          CancelIo(pipeHandle);
          GetOverlappedResult(pipeHandle, &overlapStructure, &writtenByteCount,
                              TRUE);
          MAF_LOGGER_ERROR(
              "Error while waiting for bytes to be transffered to receiver");
        } else {
          success = GetOverlappedResult(pipeHandle, &overlapStructure,
                                        &writtenByteCount, TRUE);
          if (success && (writtenByteCount == buffSize)) {
            MAF_LOGGER_INFO("Sent ", buffSize,
                            " bytes to receiver successful!");
          } else {
            // if error then it must be unsuccess
            success = false;
            MAF_LOGGER_ERROR("Could not transfer completely ", buffSize,
                             " bytes to receiver!");
          }
        }
      } else {
        MAF_LOGGER_ERROR("sending bytes failed with error: ", GetLastError());
      }
    }
  }
  return success;
}

} // namespace ipc
} // namespace messaging
} // namespace maf
