#pragma once

#include "thaf/messaging/client-server/ipc/internal/platforms/windows/NamedPipeSenderBase.h"

namespace thaf {
namespace messaging {
namespace ipc {


class  LocalIPCSenderImpl: public NamedPipeSenderBase
{
public:
    LocalIPCSenderImpl();
    ~LocalIPCSenderImpl() override;
    DataTransmissionErrorCode send(const thaf::srz::ByteArray &ba, const Address& destination = Address::INVALID_ADDRESS) override;

private:
    OVERLAPPED _oOverlap;
};

}}}
