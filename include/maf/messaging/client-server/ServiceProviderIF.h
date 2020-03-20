#pragma once

#include "ServiceMessageReceiver.h"
#include "ServiceProviderShared.h"

namespace maf {
namespace messaging {

class ServiceProviderIF : public ServiceMessageReceiver {
public:
  virtual ~ServiceProviderIF() = default;

  virtual Availability availability() const = 0;

  virtual void startServing() = 0;

  virtual void stopServing() = 0;

  virtual bool
  registerRequestHandler(const OpID &opID,
                         RequestHandlerFunction handlerFunction) = 0;

  virtual bool unregisterRequestHandler(const OpID &opID) = 0;

  virtual ActionCallStatus respondToRequest(const CSMessagePtr &csMsg) = 0;
  virtual ActionCallStatus setStatus(const OpID &propertyID,
                                     const CSMsgContentBasePtr &property) = 0;

  virtual ActionCallStatus
  broadcastSignal(const OpID &eventID, const CSMsgContentBasePtr &event) = 0;

  virtual CSMsgContentBasePtr getStatus(const OpID &propertyID) = 0;
};

} // namespace messaging
} // namespace maf
