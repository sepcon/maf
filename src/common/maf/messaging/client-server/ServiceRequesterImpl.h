#pragma once

#include <maf/messaging/client-server/Address.h>
#include <maf/messaging/client-server/CSShared.h>
#include <maf/messaging/client-server/RegID.h>
#include <maf/messaging/client-server/ServiceRequesterIF.h>
#include <maf/messaging/client-server/ServiceStatusObserverIF.h>
#include <maf/threading/Lockable.h>

#include <future>
#include <list>
#include <map>
#include <set>

namespace maf {
namespace messaging {

class ClientIF;
struct ServiceRequesterImpl {
  struct RegEntry {
    RegID::RequestIDType requestID;
    CSPayloadProcessCallback callback;

    RegEntry() = default;
    RegEntry(RegID::RequestIDType requestID, CSPayloadProcessCallback callback)
        : requestID(requestID), callback(std::move(callback)) {}
  };

  struct SyncRegEntry {
    RegID::RequestIDType requestID;
    std::promise<std::shared_ptr<CSMessage>> _msgPromise;
  };

  template <typename ValueType>
  using OpIDMap = threading::Lockable<std::map<OpID, ValueType>>;
  using RegEntriesMap = OpIDMap<std::list<RegEntry>>;
  using SyncRegEntriesMap = OpIDMap<std::list<SyncRegEntry>>;
  using CSMsgContentMap = OpIDMap<CSPayloadIFPtr>;
  using ServiceStatusObserverPtr = ServiceRequesterIF::ServiceStatusObserverPtr;
  using SyncRequestPromises = threading::Lockable<
      std::list<std::shared_ptr<std::promise<CSPayloadIFPtr>>>>;
  using ServiceStatusObservers =
      threading::Lockable<std::list<ServiceStatusObserverPtr>>;

  RegEntriesMap registerEntriesMap_;
  RegEntriesMap requestEntriesMap_;
  SyncRequestPromises syncRequestPromises_;
  ServiceStatusObservers serviceStatusObservers_;
  CSMsgContentMap propertiesCache_;
  std::weak_ptr<ClientIF> client_;
  CSIDManager idMgr_;
  ServiceID sid_;
  std::atomic<Availability> serviceStatus_ = Availability::Unavailable;

  ServiceRequesterImpl(const ServiceID &sid, std::weak_ptr<ClientIF> client);
  ~ServiceRequesterImpl();

  const ServiceID &serviceID() const { return sid_; }
  Availability serviceStatus() const noexcept;
  bool serviceUnavailable() const noexcept;

  RegID registerStatus(const OpID &propertyID,
                       CSPayloadProcessCallback callback,
                       ActionCallStatus *callStatus);

  RegID registerSignal(const OpID &eventID, CSPayloadProcessCallback callback,
                       ActionCallStatus *callStatus);

  ActionCallStatus unregister(const RegID &regID);
  ActionCallStatus unregisterAll(const OpID &propertyID);

  RegID sendRequestAsync(const OpID &opID, const CSPayloadIFPtr &msgContent,
                         CSPayloadProcessCallback callback,
                         ActionCallStatus *callStatus);

  CSPayloadIFPtr getStatus(const OpID &propertyID, ActionCallStatus *callStatus,
                           RequestTimeoutMs timeout);

  ActionCallStatus getStatus(const OpID &propertyID,
                             CSPayloadProcessCallback callback);

  CSPayloadIFPtr sendRequest(const OpID &opID, const CSPayloadIFPtr &msgContent,
                             ActionCallStatus *callStatus,
                             RequestTimeoutMs timeout);

  void abortAction(const RegID &regID, ActionCallStatus *callStatus);

  void registerServiceStatusObserver(
      ServiceStatusObserverPtr serviceStatusObserver);

  void unregisterServiceStatusObserver(
      const ServiceStatusObserverPtr &serviceStatusObserver);

  bool onIncomingMessage(const CSMessagePtr &csMsg);

  void onServiceStatusChanged(const ServiceID &sid, Availability oldStatus,
                              Availability newStatus);
  void forwardServiceStatusToObservers(const ServiceID &sid,
                                       Availability oldStatus,
                                       Availability newStatus);

  RegID registerNotification(const OpID &opID, OpCode opCode,
                             CSPayloadProcessCallback callback,
                             ActionCallStatus *callStatus);

  // Helper functions
  RegID sendMessageAsync(const OpID &operationID, OpCode operationCode,
                         const CSPayloadIFPtr &msgContent,
                         CSPayloadProcessCallback callback,
                         ActionCallStatus *callStatus);

  CSPayloadIFPtr sendMessageSync(const OpID &operationID, OpCode opCode,
                                 const CSPayloadIFPtr &msgContent,
                                 ActionCallStatus *callStatus,
                                 RequestTimeoutMs timeout);

  CSMessagePtr createCSMessage(const OpID &opID, OpCode opCode,
                               const CSPayloadIFPtr &msgContent = nullptr);

  bool onNotification(const CSMessagePtr &msg);
  void onRequestResult(const CSMessagePtr &msg);
  void abortAllSyncRequest();
  void clearAllAsyncRequests();
  void clearAllRegisterEntries();
  ActionCallStatus sendMessageToServer(const CSMessagePtr &outgoingMsg);

  RegID storeAndSendRequestToServer(RegEntriesMap &regEntriesMap,
                                    const CSMessagePtr &outgoingMsg,
                                    CSPayloadProcessCallback callback,
                                    ActionCallStatus *callStatus);

  size_t storeRegEntry(RegEntriesMap &regInfoEntries, const OpID &propertyID,
                       CSPayloadProcessCallback callback, RegID &regID);

  size_t removeRegEntry(RegEntriesMap &regInfoEntries, const RegID &regID);

  void removeRequestPromies(
      const std::shared_ptr<std::promise<CSPayloadIFPtr>> &promise);

  CSPayloadIFPtr getCachedProperty(const OpID &propertyID) const;
  void cachePropertyStatus(const OpID &propertyID, CSPayloadIFPtr &&property);
  void removeCachedProperty(const OpID &propertyID);
};

}  // namespace messaging
}  // namespace maf
