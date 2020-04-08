#pragma once

#include <maf/messaging/client-server/ServerIF.h>
#include <maf/messaging/client-server/ServiceProviderIF.h>
#include <maf/threading/Lockable.h>
#include <map>

namespace maf {
namespace messaging {

class ServerBase : public ServerIF {
public:
  // Drived class must provide implementation for this method
  ActionCallStatus sendMessageToClient(const CSMessagePtr &msg,
                                       const Address &addr) override = 0;
  virtual void notifyServiceStatusToClient(const ServiceID &sid,
                                           Availability oldStatus,
                                           Availability newStatus) = 0;

  bool registerServiceProvider(const ServiceProviderIFPtr &provider) override;
  bool unregisterServiceProvider(const ServiceProviderIFPtr &Provider) override;
  bool unregisterServiceProvider(const ServiceID &sid) override;
  bool hasServiceProvider(const ServiceID &sid) override;

  virtual bool init(const Address &serverAddr) override;
  bool deinit() override;

protected:
  bool onIncomingMessage(const CSMessagePtr &csMsg) override;
  using ProviderMap =
      threading::Lockable<std::map<ServiceID, ServiceProviderIFPtr>>;
  ProviderMap providers_;
};

} // namespace messaging
} // namespace maf
