#pragma once

#include "thaf/messaging/client-server/ipc/internal/platforms/windows/LocalIPCSender.h"
#include <windows.h>

namespace thaf {
namespace messaging {
namespace ipc {


class NamedPipeSenderBase : public IPCSender
{
public:
    ~NamedPipeSenderBase() override;
    void initConnection(const Address &addr) override;
    DataTransmissionErrorCode send(const thaf::srz::ByteArray &ba) override;
    const Address& receiverAddress() const override;
    Availability checkReceiverStatus() const override;

protected:
    std::string _pipeName;
    Address _receiverAddress;
};

}}}
