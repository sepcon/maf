#pragma once

#include "maf/messaging/client-server/ClientBase.h"
#include "BytesCommunicator.h"
#include "IPCTypes.h"
#include <thread>

namespace maf {
namespace messaging {
namespace ipc {

class IPCClientBase : public ClientBase, public BytesCommunicator
{
public:
    IPCClientBase();
    ~IPCClientBase() override;

    void init(IPCType type, const Address& serverAddress, long long serverStatusCheckPeriodMS = 1000);
    void deinit();
    DataTransmissionErrorCode sendMessageToServer(const CSMessagePtr& msg)  override;
    void onServerStatusChanged(Availability oldStatus, Availability newStatus) override;
protected:
    void monitorServerStatus(long long serverStatusCheckPeriodMS);
    std::thread _serverMonitorThread;
    std::atomic_bool _stop;
};



} // ipc
} // messaging
} // maf
