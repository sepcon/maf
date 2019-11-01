#pragma once

#include <maf/messaging/client-server/ClientBase.h>
#include "BytesCommunicator.h"
#include "IPCTypes.h"
#include <thread>

namespace maf {
namespace messaging {
namespace ipc {

class IPCClientBase : public ClientBase, public BytesCommunicator
{
public:
    IPCClientBase(IPCType type);
    ~IPCClientBase() override;

    bool init(const Address& serverAddress, long long sersverMonitoringCycleMS = 1000) override;
    bool deinit() override;
    DataTransmissionErrorCode sendMessageToServer(const CSMessagePtr& msg)  override;
    void onServerStatusChanged(Availability oldStatus, Availability newStatus) override;

protected:
    void monitorServerStatus(long long sersverMonitoringCycleMS);
    std::thread _serverMonitorThread;
    std::atomic_bool _stop;
};



} // ipc
} // messaging
} // maf
