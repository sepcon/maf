#pragma once

#include <maf/messaging/client-server/ipc/IPCReceiver.h>

namespace maf {
namespace messaging {
namespace ipc {

class LocalIPCReceiver : public IPCReceiver, BytesComeObserver
{
public:
    LocalIPCReceiver();
    ~LocalIPCReceiver() override;
    bool initConnection(const Address &address, bool isClientMode = false) override;
    bool startListening() override;
    bool stopListening() override;
    bool listening() const override;
    const Address& address() const override;

private:
    void onBytesCome(const std::shared_ptr<srz::ByteArray>& bytes) override;
    std::unique_ptr<IPCReceiver> _impl;
};
} // ipc
} // messaging
} // maf

