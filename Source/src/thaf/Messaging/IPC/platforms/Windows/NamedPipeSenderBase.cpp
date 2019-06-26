#include "thaf/Messaging/IPC/Address.h"
#include "thaf/Utils/Serialization/ByteArray.h"
#include "thaf/Messaging/IPC/Prv/Platforms/Windows/NamedPipeSenderBase.h"
#include "thaf/Messaging/IPC/Prv/Platforms/Windows/PipeShared.h"
#include "thaf/Utils/Debugging/Debug.h"
#include <windows.h>


namespace thaf {
namespace messaging {
namespace ipc {

NamedPipeSenderBase::~NamedPipeSenderBase() {}

void NamedPipeSenderBase::initConnection(const Address& addr)
{
    if(addr != Address::INVALID_ADDRESS && _receiverAddress != addr)
    {
        _receiverAddress = addr;
        _pipeName = constructPipeName(addr);
    }
}

ConnectionErrorCode NamedPipeSenderBase::send(const srz::ByteArray& /*ba*/)
{
    thafErr("Derived class must override this function[NamedPipeSenderBase::send]");
    return ConnectionErrorCode::Failed;
}

const Address &NamedPipeSenderBase::receiverAddress() const
{
    return _receiverAddress;
}

ConnectionStatus NamedPipeSenderBase::checkServerStatus() const
{
    return WaitNamedPipeA(_pipeName.c_str(), WAIT_DURATION_MAX) ? ConnectionStatus::Available : ConnectionStatus::UnAvailable;
}

}}}
