#include <maf/utils/debugging/Debug.h>
#include "LocalIPCReceiverImpl.h"
#include "PipeShared.h"
#include <maf/messaging/client-server/Connection.h>
//#include <strsafe.h>

#define CONNECTING_STATE 0
#define READING_STATE 1
#define MAX_INSTANCES 5
#define PIPE_TIMEOUT 5000

namespace maf {
namespace messaging {
namespace ipc {

#define _2dw(value) static_cast<DWORD>(value)

using PipeInstances = LocalIPCReceiverImpl::PipeInstances;
static bool connectToNewClient(HANDLE, LPOVERLAPPED);
static size_t fillbuffer(HANDLE pipeHandle, OVERLAPPED& overlapStructure, char* buffer, size_t buffSize)
{
    bool fSuccess = false;
    size_t totalBytesRead = 0;
    do
    {
        DWORD bytesRead = 0;
        fSuccess = ReadFile(
            pipeHandle,
            buffer + totalBytesRead,
            _2dw(buffSize - totalBytesRead),
            &bytesRead,
            &overlapStructure);

        if (fSuccess && bytesRead != 0)
        {
			totalBytesRead += bytesRead;
            break;
        }
        else
        {
            fSuccess = GetOverlappedResult(pipeHandle, &overlapStructure, &bytesRead, true);
			totalBytesRead += bytesRead;
        }

    } while(!fSuccess && GetLastError() == ERROR_MORE_DATA);

    return totalBytesRead ;
}
LocalIPCReceiverImpl::LocalIPCReceiverImpl()
{
}

bool LocalIPCReceiverImpl::stopListening()
{
    _stopped.store(true, std::memory_order_release);
    for(auto& hE : _hEvents)
    {
        SetEvent(hE);
    }
    waitForWorkerThreadToStop();
    return true;
}

bool LocalIPCReceiverImpl::initPipes()
{
    int pipeCount = _isClient ? MAX_INSTANCES : MAX_INSTANCES;
    for (int i = 0; i < pipeCount; ++i)
    {
        _pipeInstances.push_back(std::make_unique<PipeInstance>());
    }

    // The initial loop creates several instances of a named pipe
    // along with an event object for each instance.  An
    // overlapped ConnectNamedPipe operation is started for
    // each instance.
    for (size_t index = 0; index < _pipeInstances.size(); index++)
    {

        // Create an event object for this instance.

        _hEvents.push_back(CreateEvent(
            nullptr,    // default security attribute
            TRUE,    // manual-reset event
            TRUE,    // initial state = signaled
            nullptr));   // unnamed event object

        if (_hEvents.back() == nullptr)
        {
            mafErr("CreateEvent failed with " << GetLastError());
            return false;
        }

        _pipeInstances[index]->oOverlap.hEvent = _hEvents.back();

        _pipeInstances[index]->hPipeInst = CreateNamedPipeA(
            _pipeName.c_str(),          // pipe name
            PIPE_ACCESS_INBOUND |		// Read only
            FILE_FLAG_OVERLAPPED,       // overlapped mode
            PIPE_TYPE_MESSAGE |			// * must use PIPE_TYPE_MESSAGE conjunction to PIPE_READMODE_MESSAGE for transferring
            PIPE_READMODE_MESSAGE |     // * block of bytes that greater than buffer_size
            PIPE_WAIT,                  // blocking mode
            static_cast<DWORD>(_pipeInstances.size()),		// number of instances
            0,                          // output buffer size
            BUFFER_SIZE*sizeof(char),   // input buffer size
            PIPE_TIMEOUT,               // client time-out
            nullptr);                   // default security attributes

        if (_pipeInstances[index]->hPipeInst == INVALID_HANDLE_VALUE)
        {
            mafErr("CreateNamedPipe failed with " << GetLastError());
            return false;
        }

        // Call the subroutine to connect to the new client
        connectToNewClient( _pipeInstances[index]->hPipeInst, &_pipeInstances[index]->oOverlap);
    }
    return true;
}

void LocalIPCReceiverImpl::listningThreadFunction()
{
    DWORD  dwWait;

    initPipes();
    while (listening())
    {
        dwWait = WaitForMultipleObjects(
            static_cast<DWORD>(_hEvents.size()),    // number of event objects
            &_hEvents[0],		// array of event objects
            FALSE,              // does not wait for all
            INFINITE);          // waits indefinitely

        if(!listening()) { break; }

        int i = static_cast<int>(dwWait - WAIT_OBJECT_0);  // determines which pipe
        if (i < 0 || i > (static_cast<int>(_pipeInstances.size()) - 1))
        {
            mafErr("Index out of range.");
            return;
        }

        size_t index = static_cast<size_t>(i);
        if (readOnPipe(index))
        {
            notifyObervers(std::make_shared<srz::ByteArray>(std::move(_pipeInstances[index]->ba)));
        }
        else
        {
            mafWarn("Read nothing, GLE = " << GetLastError() << "-->" << _pipeInstances[index]->ba << "<--");
        }

        disconnectAndReconnect(index);
    }
}

bool LocalIPCReceiverImpl::readOnPipe(size_t index)
{
    bool fSuccess = false;
    auto& incommingBA = _pipeInstances[index]->ba;
    size_t bytesRead = 0;
    if(incommingBA.empty()) // read the written bytes count first
    {
        uint32_t totalComingBytes = 0;
        bytesRead = fillbuffer(
            _pipeInstances[index]->hPipeInst,
            _pipeInstances[index]->oOverlap,
            reinterpret_cast<char*>(&totalComingBytes),
            sizeof(totalComingBytes)
            );

        if(bytesRead == sizeof(totalComingBytes) )
        {
            incommingBA.resize(totalComingBytes);
            bytesRead = fillbuffer(
                _pipeInstances[index]->hPipeInst,
                _pipeInstances[index]->oOverlap,
                incommingBA.firstpos(),
                incommingBA.size()
                );
            fSuccess = (incommingBA.size() == bytesRead);
        }
    }

    return fSuccess;
}

// DisconnectAndReconnect(DWORD)
// This function is called when an error occurs or when the client
// closes its handle to the pipe. Disconnect from this client, then
// call ConnectNamedPipe to wait for another client to connect.

void LocalIPCReceiverImpl::disconnectAndReconnect(size_t index)
{
    // Disconnect the pipe instance.

    if (! DisconnectNamedPipe(_pipeInstances[index]->hPipeInst) )
    {
        mafErr("DisconnectNamedPipe failed with" << GetLastError());
    }

    // Call a subroutine to connect to the new client.
    connectToNewClient( _pipeInstances[index]->hPipeInst, &_pipeInstances[index]->oOverlap);
}

// ConnectToNewClient(HANDLE, LPOVERLAPPED)
// This function is called to start an overlapped connect operation.
// It returns TRUE if an operation is pending or FALSE if the
// connection has been completed.

bool connectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo)
{
    bool fConnected, fPendingIO = FALSE;

    // Start an overlapped connection for this pipe instance.
    fConnected = ConnectNamedPipe(hPipe, lpo);

    // Overlapped ConnectNamedPipe should return zero.
    if (fConnected)
    {
        mafErr("ConnectNamedPipe failed with " << GetLastError());
        return true;
    }

    switch (GetLastError())
    {
    // The overlapped connection in progress.
    case ERROR_IO_PENDING:
        fPendingIO = TRUE;
        break;

        // Client is already connected, so signal an event.

    case ERROR_PIPE_CONNECTED:
        if (SetEvent(lpo->hEvent))
            break;

        // If an error occurs during the connect operation...
        [[fallthrough]]; default:
    {
        mafErr("ConnectNamedPipe failed with " << GetLastError());
        return false;
    }
    }

    return fPendingIO;
}

} // ipc
} // messaging
} // maf




