#pragma once

#include "CSMessageReceiverIF.h"
#include "CSStatus.h"
#include "ServerStatusObserverIF.h"
#include "ServiceStatusObserverIF.h"

namespace maf {
namespace messaging {

using ServiceRequesterIFPtr = std::shared_ptr<class ServiceRequesterIF>;

class ClientIF : public CSMessageReceiverIF,
                 private ServiceStatusObserverIF,
                 private ServerStatusObserverIF {
public:
  virtual ~ClientIF() = default;
  virtual ActionCallStatus sendMessageToServer(const CSMessagePtr &msg) = 0;
  virtual bool hasServiceRequester(const ServiceID &sid) = 0;
  virtual ServiceRequesterIFPtr getServiceRequester(const ServiceID &sid) = 0;
  virtual Availability getServiceStatus(const ServiceID &sid) = 0;
  virtual bool init(const Address &serverAddr) = 0;
  virtual bool start() = 0;
  virtual void stop() = 0;
  virtual void deinit() = 0;
};

} // namespace messaging
} // namespace maf
