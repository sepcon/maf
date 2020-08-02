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
struct ServiceRequester : public ServiceRequesterIF {
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

  ServiceRequester(const ServiceID &sid, std::weak_ptr<ClientIF> client);
  ~ServiceRequester();

  const ServiceID &serviceID() const override { return sid_; }
  Availability serviceStatus() const noexcept override;
  bool serviceUnavailable() const noexcept;

  RegID registerStatus(const OpID &propertyID,
                       CSPayloadProcessCallback callback,
                       ActionCallStatus *callStatus) override;

  RegID registerSignal(const OpID &eventID, CSPayloadProcessCallback callback,
                       ActionCallStatus *callStatus) override;

  ActionCallStatus unregister(const RegID &regID) override;
  ActionCallStatus unregisterAll(const OpID &propertyID) override;

  RegID sendRequestAsync(const OpID &opID, const CSPayloadIFPtr &msgContent,
                         CSPayloadProcessCallback callback,
                         ActionCallStatus *callStatus) override;

  CSPayloadIFPtr getStatus(const OpID &propertyID, ActionCallStatus *callStatus,
                           RequestTimeoutMs timeout) override;

  ActionCallStatus getStatus(const OpID &propertyID,
                             CSPayloadProcessCallback callback) override;

  CSPayloadIFPtr sendRequest(const OpID &opID, const CSPayloadIFPtr &msgContent,
                             ActionCallStatus *callStatus,
                             RequestTimeoutMs timeout) override;

  void abortRequest(const RegID &regID, ActionCallStatus *callStatus) override;

  void registerServiceStatusObserver(
      ServiceStatusObserverPtr serviceStatusObserver) override;

  void unregisterServiceStatusObserver(
      const ServiceStatusObserverPtr &serviceStatusObserver) override;

  bool onIncomingMessage(const CSMessagePtr &csMsg) override;

  void onServiceStatusChanged(const ServiceID &sid, Availability oldStatus,
                              Availability newStatus) override;

 private:
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

  bool onRegistersUpdated(const CSMessagePtr &msg);
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

  size_t removeRegEntry(RegEntriesMap &regInfoEntriesMap, const RegID &regID);

  void removeRequestPromies(
      const std::shared_ptr<std::promise<CSPayloadIFPtr>> &promise);

  CSPayloadIFPtr getCachedProperty(const OpID &propertyID) const;
  void cachePropertyStatus(const OpID &propertyID, CSPayloadIFPtr &&property);
  void removeCachedProperty(const OpID &propertyID);
  bool cachedPropertyUpToDate(const OpID &propertyID) const;

  RegEntriesMap registerEntriesMap_;
  RegEntriesMap requestEntriesMap_;
  SyncRequestPromises syncRequestPromises_;
  ServiceStatusObservers serviceStatusObservers_;
  CSMsgContentMap propertiesCache_;
  std::weak_ptr<ClientIF> client_;
  CSIDManager idMgr_;
  ServiceID sid_;
  std::atomic<Availability> serviceStatus_ = Availability::Unavailable;
};

}  // namespace messaging
}  // namespace maf
