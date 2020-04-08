#pragma once

#include <maf/messaging/client-server/ClientIF.h>
#include <maf/messaging/client-server/ServiceRequesterIF.h>
#include <maf/threading/Lockable.h>
#include <map>

namespace maf {
namespace messaging {

class ClientBase : public ClientIF,
                   public std::enable_shared_from_this<ClientBase> {
public:
  // Dervied class must provide implementation for this method
  ActionCallStatus sendMessageToServer(const CSMessagePtr &msg) override = 0;
  void onServerStatusChanged(Availability oldStatus,
                             Availability newStatus) noexcept override;
  void onServiceStatusChanged(const ServiceID &sid, Availability oldStatus,
                              Availability newStatus) noexcept override;
  bool hasServiceRequester(const ServiceID &sid) override;
  ServiceRequesterIFPtr getServiceRequester(const ServiceID &sid) override;

  Availability getServiceStatus(const ServiceID &sid) override;

  bool init(const Address &serverAddress,
            long long sersverMonitoringCycleMS) override;

  bool deinit() override;

protected:
  bool onIncomingMessage(const CSMessagePtr &msg) override;
  void storeServiceStatus(const ServiceID &sid, Availability status);

  using ServiceStatusMap =
      threading::Lockable<std::map<ServiceID, Availability>>;
  using ProxyMap =
      threading::Lockable<std::map<ServiceID, ServiceRequesterIFPtr>>;
  ProxyMap _requestersMap;
  ServiceStatusMap _serviceStatusMap;
};

} // namespace messaging
} // namespace maf
