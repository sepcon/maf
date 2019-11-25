#pragma once

#include <maf/messaging/client-server/CSDefines.h>
#include <maf/messaging/client-server/internal/CSShared.h>
#include <maf/messaging/client-server/ServiceStatusObserverInterface.h>
#include <maf/messaging/client-server/RegID.h>
#include <maf/messaging/client-server/Address.h>
#include <maf/threading/Lockable.h>
#include <future>
#include <map>
#include <list>
#include <set>

namespace maf {
namespace messaging {

class ClientInterface;
struct ServiceRequesterImpl
{
    struct RegEntry
    {
        RegID::RequestIDType requestID;
        CSMessageContentHandlerCallback callback;

        RegEntry() = default;
        RegEntry(
            RegID::RequestIDType requestID,
            CSMessageContentHandlerCallback callback
            )
            : requestID(requestID), callback(std::move(callback)){}

    };

    struct SyncRegEntry
    {
        RegID::RequestIDType requestID;
        std::promise< std::shared_ptr<CSMessage> > _msgPromise;
    };

    template <typename ValueType>
    using OpIDMap                    = threading::Lockable<std::map<OpID, ValueType>>;
    using RegEntriesMap              = OpIDMap<std::list<RegEntry>>;
    using SyncRegEntriesMap          = OpIDMap<std::list<SyncRegEntry>>;
    using CSMsgContentMap            = OpIDMap<CSMsgContentBasePtr>;
    using ServiceStatusObserverIFPtr = std::weak_ptr<ServiceStatusObserverInterface>;
    using SyncRequestPromises        = threading::Lockable<std::list<std::shared_ptr<std::promise<CSMsgContentBasePtr>>>>;
    using ServiceStatusObservers     = threading::Lockable<std::list<ServiceStatusObserverIFPtr>>;

    RegEntriesMap                    _registerEntriesMap;
    RegEntriesMap                    _requestEntriesMap;
    SyncRequestPromises              _syncRequestPromises;
    ServiceStatusObservers           _serviceStatusObservers;
    CSMsgContentMap                  _propertiesCache;
    std::thread                      _serviceMonitoringThread;
    std::weak_ptr<ClientInterface>   _client;
    util::IDManager                  _idMgr;
    ServiceID                        _sid;
    Availability                     _serviceStatus;
    std::atomic_bool                 _stopFlag;


    ServiceRequesterImpl(const ServiceID& sid, std::weak_ptr<ClientInterface> client);
    ~ServiceRequesterImpl();

    Availability serviceStatus() const;

    RegID registerStatus(
        const OpID& propertyID,
        CSMessageContentHandlerCallback callback
        );

    RegID registerSignal(const OpID& eventID,
        CSMessageContentHandlerCallback callback
        );

    void unregisterStatus(const RegID& regID);
    void unregisterStatusAll(const OpID& propertyID);

    RegID getStatusAsync(
        const OpID& propertyID,
        CSMessageContentHandlerCallback callback
        );

    RegID sendRequestAsync(
        const OpID& opID,
        const CSMsgContentBasePtr& msgContent,
        CSMessageContentHandlerCallback callback
        );

    CSMsgContentBasePtr getStatus(
        const OpID& propertyID,
        unsigned long maxWaitTimeMs
        );

    CSMsgContentBasePtr sendRequest(
        const OpID& opID,
        const CSMsgContentBasePtr& msgContent = {},
        unsigned long maxWaitTimeMs = maf_INFINITE_WAIT_PERIOD
        );

    void abortAction(const RegID& regID);

    void addServiceStatusObserver(
        ServiceStatusObserverIFPtr serviceStatusObserver
        );

    void removeServiceStatusObserver(
        const ServiceStatusObserverIFPtr& serviceStatusObserver
        );

    bool onIncomingMessage(const CSMessagePtr& csMsg);

    void onServerStatusChanged(
        Availability oldStatus,
        Availability newStatus
        );
    void onServiceStatusChanged(
        const ServiceID& sid,
        Availability oldStatus,
        Availability newStatus
        );
    void forwardServerStatusToObservers(
        Availability oldStatus,
        Availability newStatus
        );
    void forwardServiceStatusToObservers(
        const ServiceID& sid,
        Availability oldStatus,
        Availability newStatus
        );

    RegID registerNotification(
        const OpID& opID,
        OpCode opCode,
        CSMessageContentHandlerCallback callback
        );

    //Helper functions
    RegID sendMessageAsync(
        const OpID& operationID,
        OpCode operationCode,
        const CSMsgContentBasePtr& msgContent = {},
        CSMessageContentHandlerCallback callback = {}
        );

    CSMsgContentBasePtr sendMessageSync(
        const OpID& operationID,
        OpCode opCode,
        const CSMsgContentBasePtr& msgContent = {},
        unsigned long maxWaitTimeMs = maf_INFINITE_WAIT_PERIOD
        );

    CSMessagePtr createCSMessage(
        const OpID& opID,
        OpCode opCode,
        const CSMsgContentBasePtr& msgContent = nullptr
        );
    void onPropChangeUpdate(const CSMessagePtr& msg);
    void onRequestResult(const CSMessagePtr& msg);
    void abortAllSyncRequest();
    void clearAllAsyncRequests();
    void clearAllRegisterEntries();
    ActionCallStatus sendMessageToServer(const CSMessagePtr& outgoingMsg);

    RegID storeAndSendRequestToServer(
        RegEntriesMap& regEntriesMap,
        const CSMessagePtr& outgoingMsg,
        CSMessageContentHandlerCallback callback
        );

    size_t storeRegEntry(
        RegEntriesMap& regInfoEntries,
        const OpID& propertyID,
        CSMessageContentHandlerCallback callback,
        RegID &regID
        );

    size_t removeRegEntry(
        RegEntriesMap& regInfoEntries,
        const RegID &regID
        );

    void removeRequestPromies(
        const std::shared_ptr<std::promise<CSMsgContentBasePtr>>& promise
        );

    CSMsgContentBasePtr getCachedProperty(const OpID& propertyID) const;
    void cachePropertyStatus(const OpID& propertyID, CSMsgContentBasePtr property);
    void removeCachedProperty(const OpID& propertyID);
};

}// messaging
}// maf

