#pragma once

#include <chrono>

#include "CSMessageReceiverIF.h"
#include "CSShared.h"
#include "RegID.h"
#include "ServiceStatusObserverIF.h"

namespace maf {
namespace messaging {

using RequestTimeoutMs = std::chrono::milliseconds;

class MAF_EXPORT ServiceRequesterIF : public CSMessageReceiverIF,
                                      public ServiceStatusObserverIF {
 public:
  using ServiceStatusObserverPtr = std::shared_ptr<ServiceStatusObserverIF>;

  virtual const ServiceID &serviceID() const = 0;

  virtual RegID registerStatus(const OpID &propertyID,
                               CSPayloadProcessCallback callback,
                               ActionCallStatus *callStatus) = 0;

  virtual RegID registerSignal(const OpID &propertyID,
                               CSPayloadProcessCallback callback,
                               ActionCallStatus *callStatus) = 0;

  virtual ActionCallStatus unregister(const RegID &regID) = 0;
  virtual ActionCallStatus unregisterAll(const OpID &propertyID) = 0;

  virtual RegID sendRequestAsync(const OpID &opID,
                                 const CSPayloadIFPtr &msgContent,
                                 CSPayloadProcessCallback callback,
                                 ActionCallStatus *callStatus) = 0;

  virtual CSPayloadIFPtr getStatus(const OpID &propertyID,
                                   ActionCallStatus *callStatus,
                                   RequestTimeoutMs timeout) = 0;

  virtual ActionCallStatus getStatus(const OpID &propertyID,
                                     CSPayloadProcessCallback callback) = 0;

  virtual CSPayloadIFPtr sendRequest(const OpID &opID,
                                     const CSPayloadIFPtr &msgContent,
                                     ActionCallStatus *callStatus,
                                     RequestTimeoutMs timeout) = 0;

  virtual void abortRequest(const RegID &regID,
                            ActionCallStatus *callStatus) = 0;

  virtual Availability serviceStatus() const = 0;

  virtual void registerServiceStatusObserver(
      ServiceStatusObserverPtr observer) = 0;

  virtual void unregisterServiceStatusObserver(
      const ServiceStatusObserverPtr &observer) = 0;

  virtual ~ServiceRequesterIF() = default;
};

}  // namespace messaging
}  // namespace maf
