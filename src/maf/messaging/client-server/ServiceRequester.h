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

  Availability serviceStatus() const override;

  RegID registerStatus(const OpID &propertyID,
                       CSMessageContentHandlerCallback callback,
                       ActionCallStatus *callStatus) override;

  RegID registerSignal(const OpID &propertyID,
                       CSMessageContentHandlerCallback callback,
                       ActionCallStatus *callStatus) override;

  ActionCallStatus unregisterBroadcast(const RegID &regID) override;
  ActionCallStatus unregisterBroadcastAll(const OpID &propertyID) override;

  RegID sendRequestAsync(const OpID &opID,
                         const CSMsgContentBasePtr &msgContent,
                         CSMessageContentHandlerCallback callback,
                         ActionCallStatus *callStatus) override;

  CSMsgContentBasePtr getStatus(const OpID &propertyID,
                                ActionCallStatus *callStatus,
                                RequestTimeoutMs timeout) override;

  ActionCallStatus getStatus(const OpID &propertyID,
                             CSMessageContentHandlerCallback callback) override;

  CSMsgContentBasePtr sendRequest(const OpID &opID,
                                  const CSMsgContentBasePtr &msgContent,
                                  ActionCallStatus *callStatus,
                                  RequestTimeoutMs timeout) override;

  void abortAction(const RegID &regID, ActionCallStatus *callStatus) override;

  void registerServiceStatusObserver(
      std::weak_ptr<ServiceStatusObserverIF> serviceStatusObserver) override;

  void unregisterServiceStatusObserver(
      const std::weak_ptr<ServiceStatusObserverIF> &serviceStatusObserver)
      override;

private:
  bool onIncomingMessage(const CSMessagePtr &csMsg) override;
  void onServiceStatusChanged(const ServiceID &sid, Availability oldStatus,
                              Availability newStatus) noexcept override;
};

} // namespace messaging
} // namespace maf
