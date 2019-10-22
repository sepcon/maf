#pragma once

#include <maf/messaging/client-server/ipc/IPCReceiver.h>

namespace maf {
namespace messaging {
namespace ipc {


class LocalIPCReceiverImpl : public IPCReceiver
{
public:
    virtual bool initConnection(Address, bool isClientMode = false);
    virtual bool startListening();
    virtual bool stopListening();
    virtual bool listening() const;
    virtual const Address& address() const;
};



}
}
}
