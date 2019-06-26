#pragma once

#include "IPCCommunicator.h"
#include "IPCMessage.h"
#include "IPCInfo.h"
#include "ClientServerContract.h"
#include "thaf/Utils/Debugging/Debug.h"
#include "thaf/Application/Component.h"
#include "thaf/Application/BasicMessages.h"

namespace thaf {
namespace messaging {
namespace ipc {

class ServiceStatusObserver
{
public:
    virtual void onStatusChanged(ConnectionStatus oldStatus, ConnectionStatus newStatus) = 0;
    virtual ~ServiceStatusObserver() = default;
};


class IPCServiceProxy : public IPCCommunicator
{
public:
    using MessageHandlerCallback = std::function<void (const std::shared_ptr<IPCMessage>&)>;


    ~IPCServiceProxy() override;

    void init(IPCType ipcType, Address receiverAddr);
    void deinit() override;

    RegID sendStatusChangeRegister(OpID propertyID, IPCServiceProxy::MessageHandlerCallback callback);
    void sendStatusChangeUnregister(RegID regID);
    void startMonitoringService(ServiceStatusObserver* observer, long checkingPeriodMs);

    RegID sendRequest
        (
            const std::shared_ptr<IPCMessage>& outgoingMsg,
            IPCServiceProxy::MessageHandlerCallback callback
            );

    void sendAbortRequest(const RegID& regID);
    void sendAbortSyncRequest(const RegID& regID);

    bool sendRequestSync
        (
            const std::shared_ptr<IPCMessage>& outgoingMsg,
            IPCServiceProxy::MessageHandlerCallback callback,
            unsigned long maxWaitTimeMs = static_cast<unsigned long>(-1)
            );


private:
    using NoPayloadIndicatorType = char;
    void onIPCMessage(const std::shared_ptr<IPCMessage>& msg) override;
    void onPropChangeUpdate(const std::shared_ptr<IPCMessage>& msg);
    void onRequestResult(const std::shared_ptr<IPCMessage>& msg);
    void onRequestSyncResult(const std::shared_ptr<IPCMessage>& msg);
    void abortAllSyncRequest();
    bool sendMessageToServer(const std::shared_ptr<IPCMessage>& outgoingMsg);
    std::shared_ptr<std::future<std::shared_ptr<IPCMessage> >> storeSyncRegEntry(const std::shared_ptr<IPCMessage>& outgoingMsg, RegID &regID);
    std::shared_ptr<std::promise<std::shared_ptr<IPCMessage> >> removeSyncRegEntry(const RegID &regID);

    RegID storeAndSendRequestToServer
        (
            RegEntriesMap& regEntriesMap,
            const std::shared_ptr<IPCMessage>& outgoingMsg,
            IPCServiceProxy::MessageHandlerCallback callback
            );
    size_t storeRegEntry
        (
            RegEntriesMap& regInfoEntries,
            OpID propertyID,
            MessageHandlerCallback callback,
            RegID &regID
            );
    size_t removeRegEntry
        (
            RegEntriesMap& regInfoEntries,
            RegID &regID
            );

private:
    RegEntriesMap _registerEntriesMap;
    RegEntriesMap _requestEntriesMap;
    SyncRegEntriesMap _syncRequestEntriesMap;
    util::IDManager _idMgr;
    std::thread _serviceMonitoringThread;
    std::atomic_bool _stopFlag;
};

}// ipc
}// messaging
}// thaf

