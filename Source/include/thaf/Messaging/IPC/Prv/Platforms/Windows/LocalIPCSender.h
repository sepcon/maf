#pragma once

#include "thaf/Messaging/IPC/IPCSender.h"


namespace thaf {
namespace messaging {
namespace ipc {

class NamedPipeSender;

class LocalIPCSender : public IPCSender
{
public:
    LocalIPCSender();
    ~LocalIPCSender() override;
    void initConnection(const thaf::messaging::ipc::Address &addr) override;
    ConnectionErrorCode send(const thaf::srz::ByteArray &ba) override;
    const Address &receiverAddress() const override;
    ConnectionStatus checkServerStatus() const override;

private:
    std::unique_ptr<IPCSender> _pImpl;
};

} // ipc
} // messaging
} // thaf

