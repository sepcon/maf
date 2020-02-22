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
    std::weak_ptr<ClientInterface>   _client;
    util::IDManager                  _idMgr;
    ServiceID                        _sid;
    std::atomic<Availability>        _serviceStatus;
    std::atomic_bool                 _stopFlag;


    ServiceRequesterImpl(const ServiceID& sid, std::weak_ptr<ClientInterface> client);
    ~ServiceRequesterImpl();

    Availability serviceStatus() const;
    bool serviceUnavailable() const;

    RegID registerStatus(
        const OpID& propertyID,
        CSMessageContentHandlerCallback callback,
        ActionCallStatus* callStatus
        );

    RegID registerSignal(const OpID& eventID,
                         CSMessageContentHandlerCallback callback,
                         ActionCallStatus* callStatus
                         );

    ActionCallStatus unregisterBroadcast(const RegID& regID);
    ActionCallStatus unregisterBroadcastAll(const OpID& propertyID);

    RegID sendRequestAsync(
        const OpID& opID,
        const CSMsgContentBasePtr& msgContent,
        CSMessageContentHandlerCallback callback,
        ActionCallStatus* callStatus
        );

    CSMsgContentBasePtr getStatus(
        const OpID& propertyID,
        unsigned long maxWaitTimeMs,
        ActionCallStatus* callStatus
        );

    CSMsgContentBasePtr sendRequest(
        const OpID& opID,
        const CSMsgContentBasePtr& msgContent,
        unsigned long maxWaitTimeMs,
        ActionCallStatus* callStatus
        );

    void abortAction(
        const RegID& regID,
        ActionCallStatus* callStatus
        );

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
        CSMessageContentHandlerCallback callback,
        ActionCallStatus* callStatus
        );

    //Helper functions
    RegID sendMessageAsync(
        const OpID& operationID,
        OpCode operationCode,
        const CSMsgContentBasePtr& msgContent,
        CSMessageContentHandlerCallback callback,
        ActionCallStatus* callStatus
        );

    CSMsgContentBasePtr sendMessageSync(
        const OpID& operationID,
        OpCode opCode,
        const CSMsgContentBasePtr& msgContent,
        unsigned long maxWaitTimeMs,
        ActionCallStatus* callStatus
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
    ActionCallStatus sendMessageToServer(
        const CSMessagePtr& outgoingMsg
        );

    RegID storeAndSendRequestToServer(
        RegEntriesMap& regEntriesMap,
        const CSMessagePtr& outgoingMsg,
        CSMessageContentHandlerCallback callback,
        ActionCallStatus* callStatus
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

