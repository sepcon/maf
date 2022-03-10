#pragma once

#include "CSMessage.h"
#include "CSMessageReceiverIF.h"
#include "CSStatus.h"
#include "ServiceStatusObserverIF.h"

namespace maf {
namespace messaging {

class ServiceProviderIF;
using ServiceProviderIFPtr = std::shared_ptr<ServiceProviderIF>;

class MAF_EXPORT ServerIF : public CSMessageReceiverIF,
                            public ServiceStatusObserverIF {
 public:
  virtual ~ServerIF() = default;
  virtual ActionCallStatus sendMessageToClient(const CSMessagePtr &msg,
                                               const Address &addr) = 0;
  virtual ServiceProviderIFPtr getServiceProvider(const ServiceID &sid) = 0;
  virtual bool hasServiceProvider(const ServiceID &sid) = 0;
  virtual bool init(const Address &serverAddr) = 0;
  virtual bool start() = 0;
  virtual void stop() = 0;
  virtual void deinit() = 0;
};

}  // namespace messaging
}  // namespace maf
