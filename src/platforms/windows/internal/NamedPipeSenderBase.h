#pragma once

#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/ipc/IPCSender.h>
#include <maf/messaging/client-server/Address.h>
#include <maf/utils/serialization/ByteArray.h>
#include "PipeShared.h"

namespace maf { using logging::Logger;
namespace messaging {
namespace ipc {


class NamedPipeSenderBase : public IPCSender
{
public:
    ~NamedPipeSenderBase() override = default;
    bool initConnection(const Address &addr) override
    {
        if(addr != Address::INVALID_ADDRESS && _receiverAddress != addr)
        {
            _receiverAddress = addr;
            _pipeName = constructPipeName(addr);
            return true;
        }
        else
        {
            return false;
        }
    }
    DataTransmissionErrorCode send(const maf::srz::ByteArray &/*ba*/, const Address& /*destination*/) override
    {
        Logger::error("Derived class must override this function[NamedPipeSenderBase::send]");
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
