#pragma once

#include <maf/SignalSlots.h>

#include "CSMessageReceiverIF.h"
#include "ServiceProviderShared.h"

namespace maf {
namespace messaging {

class MAF_EXPORT ServiceProviderIF : public CSMessageReceiverIF {
 public:
  virtual ~ServiceProviderIF() = default;

  virtual const ServiceID &serviceID() const = 0;

  virtual Availability availability() const = 0;

  virtual void startServing() = 0;

  virtual void stopServing() = 0;

  virtual void init() = 0;

  virtual void deinit() = 0;

  virtual bool registerRequestHandler(
      const OpID &opID, RequestHandlerFunction handlerFunction) = 0;

  virtual bool unregisterRequestHandler(const OpID &opID) = 0;

  virtual ActionCallStatus respondToRequest(const CSMessagePtr &csMsg) = 0;
  virtual ActionCallStatus setStatus(const OpID &propertyID,
                                     const CSPayloadIFPtr &property) = 0;
  virtual CSPayloadIFPtr getStatus(const OpID &propertyID) = 0;
  virtual ActionCallStatus removeProperty(const OpID &propertyID,
                                          bool notify = false) = 0;

  virtual ActionCallStatus broadcastSignal(const OpID &eventID,
                                           const CSPayloadIFPtr &event) = 0;
  //! register for signal or property'status change
  virtual signal_slots::Connection registerNotification(
      const OpID &opID, CSPayloadProcessCallback callback) = 0;
};

}  // namespace messaging
}  // namespace maf
