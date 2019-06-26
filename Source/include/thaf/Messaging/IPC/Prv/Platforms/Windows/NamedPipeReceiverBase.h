#pragma once

#include "thaf/Messaging/IPC/IPCReceiver.h"
#include <string>
#include <thread>
#include <atomic>

namespace thaf {
namespace messaging {
namespace ipc {


class NamedPipeReceiverBase : public IPCReceiver
{
public:
    NamedPipeReceiverBase() :  _stopped(true){}
    ~NamedPipeReceiverBase() override;
    bool initConnection(Address address, bool isClientMode = false) override;
    bool startListening() override;
    bool stopListening() override;
    bool listening() const override;
    const Address& address() const override;

protected:
    virtual void listningThreadFunction();
    void waitForWorkerThreadToStop();
    std::string _pipeName;
    std::thread _workerThread;
    Address _myaddr;
    std::atomic_bool _stopped;
	bool _isClient;
};
}
}
}
