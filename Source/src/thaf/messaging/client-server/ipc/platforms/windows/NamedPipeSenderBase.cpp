#include "thaf/messaging/client-server/Address.h"
#include "thaf/utils/serialization/ByteArray.h"
#include "thaf/messaging/client-server/ipc/internal/platforms/windows/NamedPipeSenderBase.h"
#include "thaf/messaging/client-server/ipc/internal/platforms/windows/PipeShared.h"
#include "thaf/utils/debugging/Debug.h"
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

DataTransmissionErrorCode NamedPipeSenderBase::send(const srz::ByteArray& /*ba*/)
{
    thafErr("Derived class must override this function[NamedPipeSenderBase::send]");
    return DataTransmissionErrorCode::ReceiverUnavailable;
}

const Address &NamedPipeSenderBase::receiverAddress() const
{
    return _receiverAddress;
}

Availability NamedPipeSenderBase::checkReceiverStatus() const
{
    return WaitNamedPipeA(_pipeName.c_str(), WAIT_DURATION_MAX) ? Availability::Available : Availability::Unavailable;
}

}}}
