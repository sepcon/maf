#include "LocalIPCBufferReceiverImpl.h"

#include <AccCtrl.h>
#include <AclAPI.h>
#include <maf/logging/Logger.h>

#include "PipeShared.h"

#define MAX_INSTANCES 10
#define PIPE_TIMEOUT 5000

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

#define _2dw(value) static_cast<DWORD>(value)

using PipeInstances = LocalIPCBufferReceiverImpl::PipeInstances;
static bool connectToNewClient(HANDLE, LPOVERLAPPED);
static size_t fillbuffer(HANDLE pipeHandle, OVERLAPPED &overlapStructure,
                         char *buffer, size_t buffSize) {
  bool fSuccess = false;
  size_t totalBytesRead = 0;
  do {
    DWORD bytesRead = 0;
    fSuccess = ReadFile(pipeHandle, buffer + totalBytesRead,
                        _2dw(buffSize - totalBytesRead), &bytesRead,
                        &overlapStructure);

    if (fSuccess && bytesRead != 0) {
      totalBytesRead += bytesRead;
      break;
    } else {
      fSuccess =
          GetOverlappedResult(pipeHandle, &overlapStructure, &bytesRead, true);
      totalBytesRead += bytesRead;
    }

  } while (!fSuccess && GetLastError() == ERROR_MORE_DATA);

  return totalBytesRead;
}
static void disconnectAndClosePipeInstances(
    const PipeInstances &pipeInstances) {
  for (auto &instance : pipeInstances) {
    DisconnectNamedPipe(instance->hPipeInst);
    CloseHandle(instance->hPipeInst);
  }
}

static void disablePermissionRestriction(HANDLE hPipe) {
  // An ACE must be added to pipe's DACL so that client processes
  // running under low-privilege accounts are also able to change state
  // of client end of this pipe to Non-Blocking and Message-Mode.
  PACL pACL = nullptr;
  PACL pNewACL = nullptr;
  EXPLICIT_ACCESS explicit_access_list[1];
  TRUSTEE trustee[1];
  DWORD hr;

  do {
    hr = GetSecurityInfo(
        hPipe,
        SE_KERNEL_OBJECT,           // Type of object
        DACL_SECURITY_INFORMATION,  // Type of security information to retrieve
        nullptr,
        nullptr,  // Pointer that receives owner SID & group SID
        &pACL,    // Pointer that receives a pointer to the DACL
        nullptr,
        nullptr  // Pointer that receives a pointer to the SACL & security
                 // descriptr of the object
    );

    if (FAILED(hr)) {
      break;
    }

    char ptsName[] = TEXT("Everyone");
    // Identifies the user, group, or program to which the access control entry
    // (ACE) applies
    trustee[0].TrusteeForm = TRUSTEE_IS_NAME;
    trustee[0].TrusteeType = TRUSTEE_IS_GROUP;
    trustee[0].ptstrName = ptsName;
    trustee[0].MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    trustee[0].pMultipleTrustee = nullptr;

    ZeroMemory(&explicit_access_list[0], sizeof(EXPLICIT_ACCESS));
    explicit_access_list[0].grfAccessMode = GRANT_ACCESS;
    explicit_access_list[0].grfAccessPermissions =
        GENERIC_READ |
        GENERIC_WRITE;  // Make sure to use exact same permissions for client!
    explicit_access_list[0].grfInheritance = NO_INHERITANCE;
    explicit_access_list[0].Trustee = trustee[0];

    // Merge access control information to the existing ACL
    hr = SetEntriesInAcl(
        _countof(
            explicit_access_list),  // Size of array passed in the next argument
        explicit_access_list,       // Pointer to array that describe the access
                                    // control information
        pACL,                       // Pointer to existing ACL
        &pNewACL  // Pointer that receives a pointer to the new ACL
    );

    if (FAILED(hr)) {
      break;
    }

    // Sets specified security information in the security descriptor of a
    // specified object
    hr = SetSecurityInfo(
        hPipe,
        SE_KERNEL_OBJECT,           // Type of object
        DACL_SECURITY_INFORMATION,  // Type of security information to set
        nullptr,
        nullptr,  // Pointer that identifies the owner SID & group SID
        pNewACL,  // Pointer to the new DACL
        nullptr   // Pointer to the new SACL
    );

    if (FAILED(hr)) {
      break;
    }

  } while (false);

  if (pNewACL) LocalFree(pNewACL);
}

LocalIPCBufferReceiverImpl::LocalIPCBufferReceiverImpl() {}

bool LocalIPCBufferReceiverImpl::stop() {
  stopped_.store(true, std::memory_order_release);
  disconnectAndClosePipeInstances(pipeInstances_);
  for (auto &hE : hEvents_) {
    SetEvent(hE);
  }
  return true;
}

void LocalIPCBufferReceiverImpl::setObserver(BytesComeCallback &&callback) {
  bytesComeCallback_ = std::move(callback);
}

bool LocalIPCBufferReceiverImpl::init(const Address &address) {
  return Base::init(address) && initPipes();
}

bool LocalIPCBufferReceiverImpl::initPipes() {
  for (int i = 0; i < MAX_INSTANCES; ++i) {
    pipeInstances_.push_back(std::make_unique<PipeInstance>());
  }

  // The initial loop creates several instances of a named pipe
  // along with an event object for each instance.  An
  // overlapped ConnectNamedPipe operation is started for
  // each instance.
  for (auto &instance : pipeInstances_) {
    // Create an event object for this instance.

    hEvents_.push_back(CreateEvent(nullptr,    // default security attribute
                                   TRUE,       // manual-reset event
                                   TRUE,       // initial state = signaled
                                   nullptr));  // unnamed event object

    if (hEvents_.back() == nullptr) {
      MAF_LOGGER_ERROR("CreateEvent failed with ", GetLastError());
      return false;
    }

    instance->oOverlap.hEvent = hEvents_.back();

    // clang-format off
    instance->hPipeInst = CreateNamedPipeA(
        pipeName_.c_str(),          // pipe name
        PIPE_ACCESS_INBOUND  |      // Read only
        WRITE_DAC            |
        FILE_FLAG_OVERLAPPED,       // overlapped mode
        PIPE_TYPE_MESSAGE    |      // * must use PIPE_TYPE_MESSAGE conjunction to PIPE_READMODE_MESSAGE for transferring
        PIPE_READMODE_MESSAGE|      // * block of bytes that greater than buffer_size
        PIPE_WAIT,                  // blocking mode
        _2dw(pipeInstances_.size()),// number of instances
        0,                          // output buffer size
        BUFFER_SIZE*sizeof(char),   // input buffer size
        PIPE_TIMEOUT,               // client time-out
        nullptr);                   // default security attributes

    // clang-format on
    if (instance->hPipeInst == INVALID_HANDLE_VALUE) {
      MAF_LOGGER_ERROR("CreateNamedPipe failed with ", GetLastError());
      return false;
    }

    disablePermissionRestriction(instance->hPipeInst);

    // Call the subroutine to connect to the new client
    if (!connectToNewClient(instance->hPipeInst, &instance->oOverlap)) {
      return false;
    }
  }
  return true;
}

void LocalIPCBufferReceiverImpl::startListening() {
  DWORD dwWait;
  while (running()) {
    dwWait = WaitForMultipleObjects(
        static_cast<DWORD>(hEvents_.size()),  // number of event objects
        &hEvents_[0],                         // array of event objects
        FALSE,                                // does not wait for all
        INFINITE);                            // waits indefinitely

    if (!running()) {
      break;
    }

    int i = static_cast<int>(dwWait - WAIT_OBJECT_0);  // determines which pipe
    if (i < 0 || i > (static_cast<int>(pipeInstances_.size()) - 1)) {
      MAF_LOGGER_ERROR(
          "Index out of range, total pipes = ", pipeInstances_.size(),
          " but available pipe index is: ", i);
      return;
    }

    size_t index = static_cast<size_t>(i);
    if (readOnPipe(index)) {
      bytesComeCallback_(std::move(pipeInstances_[index]->ba));
    } else {
      MAF_LOGGER_WARN("Read nothing, GLE = ", GetLastError(), "-->",
                      pipeInstances_[index]->ba, "<--");
    }

    disconnectAndReconnect(index);
  }
}

bool LocalIPCBufferReceiverImpl::readOnPipe(size_t index) {
  bool fSuccess = false;
  auto &incommingBA = pipeInstances_[index]->ba;
  size_t bytesRead = 0;
  if (incommingBA.empty())  // read the written bytes count first
  {
    uint32_t totalComingBytes = 0;
    bytesRead = fillbuffer(
        pipeInstances_[index]->hPipeInst, pipeInstances_[index]->oOverlap,
        reinterpret_cast<char *>(&totalComingBytes), sizeof(totalComingBytes));

    if (bytesRead == sizeof(totalComingBytes)) {
      incommingBA.resize(totalComingBytes);
      bytesRead = fillbuffer(pipeInstances_[index]->hPipeInst,
                             pipeInstances_[index]->oOverlap,
                             incommingBA.data(), incommingBA.size());
      fSuccess = (incommingBA.size() == bytesRead);
    }
  }

  return fSuccess;
}

// DisconnectAndReconnect(DWORD)
// This function is called when an error occurs or when the client
// closes its handle to the pipe. Disconnect from this client, then
// call ConnectNamedPipe to wait for another client to connect.

void LocalIPCBufferReceiverImpl::disconnectAndReconnect(size_t index) {
  // Disconnect the pipe instance.

  if (!DisconnectNamedPipe(pipeInstances_[index]->hPipeInst)) {
    MAF_LOGGER_ERROR("DisconnectNamedPipe failed with", GetLastError());
  }

  // Call a subroutine to connect to the new client.
  connectToNewClient(pipeInstances_[index]->hPipeInst,
                     &pipeInstances_[index]->oOverlap);
}

// ConnectToNewClient(HANDLE, LPOVERLAPPED)
// This function is called to start an overlapped connect operation.
// It returns TRUE if an operation is pending or FALSE if the
// connection has been completed.

bool connectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo) {
  bool fConnected, fPendingIO = FALSE;

  // Start an overlapped connection for this pipe instance.
  fConnected = ConnectNamedPipe(hPipe, lpo);

  // Overlapped ConnectNamedPipe should return zero.
  if (fConnected) {
    MAF_LOGGER_ERROR("ConnectNamedPipe failed with ", GetLastError());
    return true;
  }

  switch (GetLastError()) {
    // The overlapped connection in progress.
    case ERROR_IO_PENDING:
      fPendingIO = TRUE;
      break;

      // Client is already connected, so signal an event.

    case ERROR_PIPE_CONNECTED:
      if (SetEvent(lpo->hEvent)) break;

      // If an error occurs during the connect operation...
      [[fallthrough]];
    default: {
      MAF_LOGGER_ERROR("ConnectNamedPipe failed with ", GetLastError());
      return false;
    }
  }

  return fPendingIO;
}

}  // namespace local
}  // namespace ipc
}  // namespace messaging
}  // namespace maf
