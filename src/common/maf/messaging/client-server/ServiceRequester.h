#pragma once

#include <maf/messaging/client-server/ServiceRequesterIF.h>

namespace maf {
namespace messaging {

class ClientIF;
struct ServiceRequesterImpl;

class ServiceRequester : public ServiceRequesterIF {
  std::unique_ptr<ServiceRequesterImpl> pImpl_;

 public:
  ServiceRequester(const ServiceID &sid, std::weak_ptr<ClientIF> client);
  ~ServiceRequester() override;

  const ServiceID &serviceID() const override;

  Availability serviceStatus() const override;

  RegID registerStatus(const OpID &propertyID,
                       CSPayloadProcessCallback callback,
                       ActionCallStatus *callStatus) override;

  RegID registerSignal(const OpID &propertyID,
                       CSPayloadProcessCallback callback,
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

  void abortAction(const RegID &regID, ActionCallStatus *callStatus) override;

  void registerServiceStatusObserver(
      ServiceStatusObserverPtr serviceStatusObserver) override;

  void unregisterServiceStatusObserver(
      const ServiceStatusObserverPtr &serviceStatusObserver) override;

 private:
  bool onIncomingMessage(const CSMessagePtr &csMsg) override;
  void onServiceStatusChanged(const ServiceID &sid, Availability oldStatus,
                              Availability newStatus) noexcept override;
};

}  // namespace messaging
}  // namespace maf
