#pragma once

#include "thaf/messaging/ipc/prv/platforms/windows/NamedPipeSenderBase.h"

namespace thaf {
namespace messaging {
namespace ipc {


class  OverlappedPipeSender: public NamedPipeSenderBase
{
public:
    OverlappedPipeSender();
    ~OverlappedPipeSender() override;
    DataTransmissionErrorCode send(const thaf::srz::ByteArray &ba) override;

private:
    HANDLE openPipe();
    OVERLAPPED _oOverlap;
};

}}}
