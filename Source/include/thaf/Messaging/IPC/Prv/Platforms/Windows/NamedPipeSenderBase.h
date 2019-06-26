#pragma once

#include "thaf/Messaging/IPC/Prv/Platforms/Windows/LocalIPCSender.h"
#include "thaf/Messaging/IPC/Address.h"
#include <windows.h>

namespace thaf {
namespace messaging {
namespace ipc {


class NamedPipeSenderBase : public IPCSender
{
public:
    ~NamedPipeSenderBase() override;
    void initConnection(const thaf::messaging::ipc::Address &addr) override;
    ConnectionErrorCode send(const thaf::srz::ByteArray &ba) override;
    const Address& receiverAddress() const override;
    ConnectionStatus checkServerStatus() const override;

protected:
    std::string _pipeName;
    Address _receiverAddress;
};

}}}
