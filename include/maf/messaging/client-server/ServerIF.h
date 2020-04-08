#pragma once

#include "CSMessage.h"
#include "CSMessageReceiverIF.h"
#include "CSStatus.h"

namespace maf {
namespace messaging {

class ServiceProviderIF;
using ServiceProviderIFPtr = std::shared_ptr<ServiceProviderIF>;

class ServerIF : public CSMessageReceiverIF {
public:
  virtual ~ServerIF() = default;
  virtual ActionCallStatus sendMessageToClient(const CSMessagePtr &msg,
                                               const Address &addr) = 0;
  virtual bool
  registerServiceProvider(const ServiceProviderIFPtr &serviceProvider) = 0;
  virtual bool
  unregisterServiceProvider(const ServiceProviderIFPtr &serviceProvider) = 0;
  virtual bool unregisterServiceProvider(const ServiceID &sid) = 0;
  virtual bool hasServiceProvider(const ServiceID &sid) = 0;
  virtual bool init(const Address &serverAddr) = 0;
  virtual bool deinit() = 0;
};

} // namespace messaging
} // namespace maf
