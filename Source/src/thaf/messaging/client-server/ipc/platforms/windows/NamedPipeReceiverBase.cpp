#include "thaf/messaging/client-server/ipc/internal/platforms/windows/LocalIPCSender.h"
#include "thaf/messaging/client-server/ipc/internal/platforms/windows/NamedPipeReceiverBase.h"
#include "thaf/messaging/client-server/ipc/internal/platforms/windows/PipeShared.h"
#include "thaf/utils/serialization/ByteArray.h"
#include "thaf/utils/debugging/Debug.h"
#include <thread>
#include <windows.h>
#include <stdio.h>


namespace thaf {
namespace messaging {
namespace ipc {

NamedPipeReceiverBase::~NamedPipeReceiverBase()
{
    if(!_stopped)
    {
        stopListening();
    }

    if(_workerThread.joinable())
    {
        _workerThread.join();
    }
}

bool NamedPipeReceiverBase::initConnection(Address address, bool isClientMode)
{
	_isClient = isClientMode;
    if(isClientMode)
    {
        static std::atomic<uint16_t> receiverCount(0);
        receiverCount += 1;
        uint16_t randomPort = receiverCount;
        _myaddr = Address(address.get_name() + std::to_string(GetCurrentProcessId()), randomPort);
    }
    else
    {
        _myaddr = std::move(address);
    }
    _pipeName = constructPipeName(_myaddr);
    return true;
}

bool NamedPipeReceiverBase::startListening()
{
    if(!listening())
    {
        _stopped.store(false, std::memory_order_release);
        _workerThread = std::thread { &NamedPipeReceiverBase::listningThreadFunction, this };
    }
    return true;
}

void NamedPipeReceiverBase::listningThreadFunction() {
    thafWarn("listningThreadFunction must be overridden by derived class");
}

void NamedPipeReceiverBase::waitForWorkerThreadToStop()
{
    if(_workerThread.joinable())
    {
        _workerThread.join();
    }
}

bool NamedPipeReceiverBase::stopListening()
{
    _stopped.store(true, std::memory_order_release);
    waitForWorkerThreadToStop();
    return false;
}

bool NamedPipeReceiverBase::listening() const
{
    return !_stopped.load(std::memory_order_acquire);
}

const Address &NamedPipeReceiverBase::address() const
{
    return _myaddr;
}




} // ipc
} // messaging
} // thaf
