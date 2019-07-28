#pragma once

#include "Connection.h"
#include "interfaces/CSStatus.h"
#include "interfaces/ServiceProxyInterface.h"

namespace thaf {
namespace messaging {

class ClientInterface;
class ServiceProxyBase : public ServiceProxyInterface
{
public:
    ServiceProxyBase(ServiceID sid, ClientInterface* client);
    ~ServiceProxyBase() override;

    RegID sendStatusChangeRegister(OpID propertyID, CSMessageHandlerCallback callback) override;
    void sendStatusChangeUnregister(RegID regID) override;
    RegID sendRequest
        (
            const CSMsgContentPtr& msgContent,
            CSMessageHandlerCallback callback
            ) override;
    void sendAbortRequest(const RegID& regID) override;
    void sendAbortSyncRequest(const RegID& regID) override;
    bool sendRequestSync
    (
            const CSMsgContentPtr& msgContent,
            CSMessageHandlerCallback callback,
            unsigned long maxWaitTimeMs = static_cast<unsigned long>(-1)
            ) override;

    CSMessagePtr sendRequestSync
    (
            const CSMsgContentPtr& msgContent,
            unsigned long maxWaitTimeMs = static_cast<unsigned long>(-1)
            ) override;

protected:
    void onServerStatusChanged(Availability oldStatus, Availability newStatus) override;
    void onServiceStatusChanged(ServiceID sid, Availability oldStatus, Availability newStatus) override;
    bool onIncomingMessage(const CSMessagePtr& msg) override;

    CSMessagePtr createCSMessage(OpID opID, OpCode opCode, const CSMsgContentPtr& msgContent = nullptr);
    void onPropChangeUpdate(const CSMessagePtr& msg);
    void onRequestResult(const CSMessagePtr& msg);
    void onRequestSyncResult(const CSMessagePtr& msg);
    void abortAllSyncRequest();
    bool sendMessageToServer(const CSMessagePtr& outgoingMsg);
    std::shared_ptr<std::future<CSMessagePtr >> storeSyncRegEntry(const CSMessagePtr& outgoingMsg, RegID &regID);
    std::shared_ptr<std::promise<CSMessagePtr >> removeSyncRegEntry(const RegID &regID);

    RegID storeAndSendRequestToServer
        (
            RegEntriesMap& regEntriesMap,
            const CSMessagePtr& outgoingMsg,
            CSMessageHandlerCallback callback
            );
    size_t storeRegEntry
        (
            RegEntriesMap& regInfoEntries,
            OpID propertyID,
            CSMessageHandlerCallback callback,
            RegID &regID
            );
    size_t removeRegEntry
        (
            RegEntriesMap& regInfoEntries,
            RegID &regID
            );

protected:
    ClientInterface* _client;
    RegEntriesMap _registerEntriesMap;
    RegEntriesMap _requestEntriesMap;
    SyncRegEntriesMap _syncRequestEntriesMap;
    util::IDManager _idMgr;
    std::thread _serviceMonitoringThread;
    std::atomic_bool _stopFlag;
};

}// messaging
}// thaf

