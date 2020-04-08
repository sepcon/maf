#pragma once

#include <chrono>

#include "RegID.h"
#include "ServiceMessageReceiver.h"
#include "ServiceStatusObserverIF.h"
#include "internal/CSShared.h"

namespace maf {
namespace messaging {

using RequestTimeoutMs = std::chrono::milliseconds;

class ServiceRequesterIF : public ServiceMessageReceiver,
                           public ServiceStatusObserverIF {
public:
  virtual RegID registerStatus(const OpID &propertyID,
                               CSMessageContentHandlerCallback callback,
                               ActionCallStatus *callStatus) = 0;

  virtual RegID registerSignal(const OpID &propertyID,
                               CSMessageContentHandlerCallback callback,
                               ActionCallStatus *callStatus) = 0;

  virtual ActionCallStatus unregisterBroadcast(const RegID &regID) = 0;
  virtual ActionCallStatus unregisterBroadcastAll(const OpID &propertyID) = 0;

  virtual RegID sendRequestAsync(const OpID &opID,
                                 const CSMsgContentBasePtr &msgContent,
                                 CSMessageContentHandlerCallback callback,
                                 ActionCallStatus *callStatus) = 0;

  virtual CSMsgContentBasePtr getStatus(const OpID &propertyID,
                                        ActionCallStatus *callStatus,
                                        RequestTimeoutMs timeout) = 0;

  virtual ActionCallStatus
  getStatus(const OpID &propertyID,
            CSMessageContentHandlerCallback callback) = 0;

  virtual CSMsgContentBasePtr sendRequest(const OpID &opID,
                                          const CSMsgContentBasePtr &msgContent,
                                          ActionCallStatus *callStatus,
                                          RequestTimeoutMs timeout) = 0;

  virtual void abortAction(const RegID &regID,
                           ActionCallStatus *callStatus) = 0;

  virtual Availability serviceStatus() const = 0;

  virtual void registerServiceStatusObserver(
      std::weak_ptr<ServiceStatusObserverIF> pServiceStatusObserver) = 0;

  virtual void unregisterServiceStatusObserver(
      const std::weak_ptr<ServiceStatusObserverIF> &pServiceStatusObserver) = 0;

  virtual ~ServiceRequesterIF() = default;
};

} // namespace messaging
} // namespace maf
