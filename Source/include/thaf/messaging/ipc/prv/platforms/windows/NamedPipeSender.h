#pragma once

#include "thaf/messaging/ipc/prv/platforms/windows/NamedPipeSenderBase.h"

namespace thaf {
namespace messaging {
namespace ipc {


class NamedPipeSender : public NamedPipeSenderBase
{
public:
    ~NamedPipeSender() override;
    DataTransmissionErrorCode send(const thaf::srz::ByteArray &ba) override;
private:
    HANDLE openPipe();
};

}}}
