#pragma once

#include "thaf/Messaging/IPC/Prv/Platforms/Windows/NamedPipeSenderBase.h"

namespace thaf {
namespace messaging {
namespace ipc {


class  OverlappedPipeSender: public NamedPipeSenderBase
{
public:
    OverlappedPipeSender();
    ~OverlappedPipeSender() override;
    ConnectionErrorCode send(const thaf::srz::ByteArray &ba) override;

private:
    HANDLE openPipe();
    OVERLAPPED _oOverlap;
};

}}}
