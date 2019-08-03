#pragma once

#include "thaf/messaging/client-server/ipc/internal/platforms/windows/PipeShared.h"
#include "thaf/messaging/client-server/ipc/IPCSender.h"
#include "thaf/messaging/client-server/Address.h"
#include "thaf/utils/serialization/ByteArray.h"
#include "thaf/utils/debugging/Debug.h"
#include <windows.h>

namespace thaf {
namespace messaging {
namespace ipc {


class NamedPipeSenderBase : public IPCSender
{
public:
    ~NamedPipeSenderBase() override = default;
    void initConnection(const Address &addr) override
    {
        if(addr != Address::INVALID_ADDRESS && _receiverAddress != addr)
        {
            _receiverAddress = addr;
            _pipeName = constructPipeName(addr);
        }
    }
    DataTransmissionErrorCode send(const thaf::srz::ByteArray &/*ba*/, const Address& /*destination*/) override
    {
        thafErr("Derived class must override this function[NamedPipeSenderBase::send]");
        return DataTransmissionErrorCode::ReceiverUnavailable;
    }
    const Address& receiverAddress() const override
    {
        return _receiverAddress;
    }
    Availability checkReceiverStatus() const override
    {
        return WaitNamedPipeA(_pipeName.c_str(), WAIT_DURATION_MAX) ? Availability::Available : Availability::Unavailable;
    }

protected:
    std::string _pipeName;
    Address _receiverAddress;
};

}}}
