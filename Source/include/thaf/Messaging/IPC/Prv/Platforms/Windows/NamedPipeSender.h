#pragma once

#include "thaf/Messaging/IPC/Prv/Platforms/Windows/NamedPipeSenderBase.h"

namespace thaf {
namespace messaging {
namespace ipc {


class NamedPipeSender : public NamedPipeSenderBase
{
public:
    ~NamedPipeSender() override;
    ConnectionErrorCode send(const thaf::srz::ByteArray &ba) override;
private:
    HANDLE openPipe();
};

}}}
