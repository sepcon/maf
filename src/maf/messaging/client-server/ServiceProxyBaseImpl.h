#pragma once

#include <maf/messaging/client-server/Connection.h>
#include <maf/messaging/client-server/CSStatus.h>
#include <maf/messaging/client-server/ServiceProxyInterface.h>
#include <maf/messaging/client-server/CSDefines.h>
#include <maf/messaging/client-server/CSMessage.h>
#include <maf/messaging/client-server/Address.h>
#include <maf/utils/cppextension/Lockable.h>
#include <future>
#include <map>
#include <list>
#include <set>

namespace maf {
namespace messaging {

class ClientInterface;
class ServiceProxyBaseImpl
{
    friend class ServiceProxyBase;
public:
    ServiceProxyBaseImpl(ServiceID sid, ClientInterface* client);
    ~ServiceProxyBaseImpl();

    RegID sendStatusChangeRegister(OpID propertyID, CSMessageHandlerCallback callback);
    void sendStatusChangeUnregister(RegID regID);
    void sendStatusChangeUnregisterAll(OpID propertyID);

    RegID sendRequest
        (
            const CSMsgContentPtr& msgContent,
            CSMessageHandlerCallback callback
            );
    void sendAbortRequest(const RegID& regID);
    void sendAbortSyncRequest(const RegID& regID);
    bool sendRequestSync
        (
            const CSMsgContentPtr& msgContent,
            CSMessageHandlerCallback callback,
            unsigned long maxWaitTimeMs = maf_INFINITE_WAIT_PERIOD
            );

    CSMessagePtr sendRequestSync
        (
            const CSMsgContentPtr& msgContent,
            unsigned long maxWaitTimeMs = maf_INFINITE_WAIT_PERIOD
            );

    void onServerStatusChanged(Availability oldStatus, Availability newStatus);
    void onServiceStatusChanged(ServiceID sid, Availability oldStatus, Availability newStatus);
    bool onIncomingMessage(const CSMessagePtr& csMsg);

    ClientInterface* getClient() { return _client; }
    const ServiceID& serviceID() const { return _sid; }
    void setServiceID(ServiceID sid) { _sid = sid; }

protected:
    struct RegEntry
    {
        RegEntry() = default;
        RegEntry(RegID::RequestIDType requestID, std::function<void (const std::shared_ptr<CSMessage>&) > callback)
            : requestID(requestID), callback(std::move(callback)){}

        RegID::RequestIDType requestID;
        std::function<void (const std::shared_ptr<CSMessage>&) > callback;
    };

    struct SyncRegEntry
    {
        RegID::RequestIDType requestID;
        std::promise< std::shared_ptr<CSMessage> > _msgPromise;
    };

    using RegEntriesMap = nstl::Lockable<std::map<OpID, std::list<RegEntry> >>;
    using SyncRegEntriesMap = nstl::Lockable<std::map<OpID, std::list<SyncRegEntry>>>;

    //Helper functions
    RegID sendRequest(OpID operationID,
                      const CSMsgContentPtr& msgContent = {},
                      CSMessageHandlerCallback callback = {});
    CSMessagePtr sendRequestSync
        (
            OpID operationID,
            const CSMsgContentPtr& msgContent = {},
            unsigned long maxWaitTimeMs = maf_INFINITE_WAIT_PERIOD
            );

    CSMessagePtr createCSMessage(OpID opID, OpCode opCode, const CSMsgContentPtr& msgContent = nullptr);
    void onPropChangeUpdate(const CSMessagePtr& msg);
    void onRequestResult(const CSMessagePtr& msg, bool done = true);
    void onRequestSyncResult(const CSMessagePtr& msg);
    void abortAllSyncRequest();
    void clearAllAsyncRequests();
    void clearAllRegisterEntries();
    bool sendMessageToServer(const CSMessagePtr& outgoingMsg);
    std::future<CSMessagePtr> storeSyncRegEntry(const CSMessagePtr& outgoingMsg, RegID &regID);
    std::shared_ptr<std::promise<CSMessagePtr >> pickOutSyncRegEntry(const RegID &regID);

    RegID storeAndSendRequestToServer
        (
            RegEntriesMap& regEntriesMap,
            const CSMessagePtr& outgoingMsg,
            CSMessageHandlerCallback callback,
            bool forceSend = true
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
    ServiceID _sid;
    ClientInterface* _client;
    RegEntriesMap _registerEntriesMap;
    RegEntriesMap _requestEntriesMap;
    SyncRegEntriesMap _syncRequestEntriesMap;
    util::IDManager _idMgr;
    std::thread _serviceMonitoringThread;
    std::atomic_bool _stopFlag;
};

}// messaging
}// maf

