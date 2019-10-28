#pragma once

#include <maf/messaging/client-server/ipc/IPCReceiver.h>
#include "SocketShared.h"
#include <future>

namespace maf {
namespace messaging {
namespace ipc {


class LocalIPCReceiverImpl : public IPCReceiver
{
public:
    ~LocalIPCReceiverImpl() override;
    bool initConnection(const Address& addr, bool isClientMode = false) override;
    bool startListening() override;
    bool stopListening() override;
    bool listening() const override;
    const Address& address() const override;
private:
    bool listeningThreadFunc(int fdMySock);
    Address  _myaddr;
    sockaddr_un _mySockAddr;
    std::thread _listeningThread;
    std::atomic_bool  _stopped = false;
    bool _isClient = false;
};



}
}
}
