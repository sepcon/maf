#pragma once

#include <maf/messaging/client-server/ServiceProviderIF.h>
#include <maf/messaging/client-server/internal/CSShared.h>

namespace maf {
namespace messaging {

class ServerIF;
struct ServiceProviderImpl;

class ServiceProvider : public ServiceProviderIF,
                        public std::enable_shared_from_this<ServiceProvider> {
public:
  ServiceProvider(ServiceID sid, std::weak_ptr<ServerIF> server);

  ~ServiceProvider() override;

  Availability availability() const override;

  bool registerRequestHandler(const OpID &opID,
                              RequestHandlerFunction handlerFunction) override;

  bool unregisterRequestHandler(const OpID &opID) override;

  ActionCallStatus respondToRequest(const CSMessagePtr &csMsg) override;

  ActionCallStatus setStatus(const OpID &propertyID,
                             const CSMsgContentBasePtr &property) override;

  ActionCallStatus broadcastSignal(const OpID &signalID,
                                   const CSMsgContentBasePtr &signal) override;

  CSMsgContentBasePtr getStatus(const OpID &propertyID) override;

  void startServing() override;
  void stopServing() override;

private:
  bool onIncomingMessage(const CSMessagePtr &csMsg) override;
  std::unique_ptr<ServiceProviderImpl> pImpl_;
};

} // namespace messaging
} // namespace maf
