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

    using ServiceStatusObserverIFPtr    = std::weak_ptr<ServiceStatusObserverInterface>;
    using RegEntriesMap                 = threading::Lockable<std::map<OpID, std::list<RegEntry> >>;
    using SyncRegEntriesMap             = threading::Lockable<std::map<OpID, std::list<SyncRegEntry>>>;
    using SyncRequestPromises           = threading::Lockable<std::list<std::shared_ptr<std::promise<CSMsgContentBasePtr>>>>;
    using ServiceStatusObservers        = threading::Lockable<std::list<ServiceStatusObserverIFPtr>>;
    using CSMsgContentMap               = threading::Lockable<std::map<OpID, CSMsgContentBasePtr> >;

    RegEntriesMap                       _registerEntriesMap;
    RegEntriesMap                       _requestEntriesMap;
    SyncRequestPromises                 _syncRequestPromises;
    ServiceStatusObservers              _serviceStatusObservers;
    CSMsgContentMap                     _propertiesCache;
    std::thread                         _serviceMonitoringThread;
    std::weak_ptr<ClientInterface>      _client;
    util::IDManager                     _idMgr;
    ServiceID                           _sid;
    Availability                        _serviceStatus;
    std::atomic_bool                    _stopFlag;


    ServiceRequesterImpl(ServiceID sid, std::weak_ptr<ClientInterface> client);
    ~ServiceRequesterImpl();

    Availability serviceStatus() const;

    RegID registerStatus(
        OpID propertyID,
        CSMessageContentHandlerCallback callback
        );
    void unregisterStatus(const RegID& regID);
    void unregisterStatusAll(OpID propertyID);

    RegID getStatusAsync(
        OpID propertyID,
        CSMessageContentHandlerCallback callback
        );

    RegID requestActionAsync(
        OpID opID,
        const CSMsgContentBasePtr& msgContent,
        CSMessageContentHandlerCallback callback
        );

    CSMsgContentBasePtr getStatus(
        OpID propertyID,
        unsigned long maxWaitTimeMs
        );

    CSMsgContentBasePtr requestAction(
        OpID opID,
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
        ServiceID sid,
        Availability oldStatus,
        Availability newStatus
        );
    void forwardServerStatusToObservers(
        Availability oldStatus,
        Availability newStatus
        );
    void forwardServiceStatusToObservers(
        ServiceID sid,
        Availability oldStatus,
        Availability newStatus
        );


    //Helper functions
    RegID sendMessageAsync(
        OpID operationID,
        OpCode operationCode,
        const CSMsgContentBasePtr& msgContent = {},
        CSMessageContentHandlerCallback callback = {}
        );

    CSMsgContentBasePtr sendMessageSync(
        OpID operationID,
        OpCode opCode,
        const CSMsgContentBasePtr& msgContent = {},
        unsigned long maxWaitTimeMs = maf_INFINITE_WAIT_PERIOD
        );

    CSMessagePtr createCSMessage(
        OpID opID,
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
        OpID propertyID,
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

    CSMsgContentBasePtr getCachedProperty(OpID propertyID) const;
    void cachePropertyStatus(OpID propertyID, CSMsgContentBasePtr property);
    void removeCachedProperty(OpID propertyID);
};

}// messaging
}// maf

