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
    DataTransmissionErrorCode send(const thaf::srz::ByteArray &ba) override;

private:
    HANDLE openPipe();
    OVERLAPPED _oOverlap;
};

}}}
