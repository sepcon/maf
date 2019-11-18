#pragma once

#include "RegID.h"
#include "ServiceStatusObserverInterface.h"
#include "ServiceMessageReceiver.h"
#include "internal/CSShared.h"

namespace maf {
namespace messaging {

class ServiceRequesterInterface:
    public ServiceMessageReceiver,
    public ServiceStatusObserverInterface
{
public:
    virtual RegID registerStatus(
            OpID propertyID,
            CSMessageContentHandlerCallback callback) = 0;
    virtual void unregisterStatus(const RegID& regID) = 0;
    virtual void unregisterStatusAll(OpID propertyID) = 0;

    virtual RegID getStatusAsync(
            OpID propertyID,
            CSMessageContentHandlerCallback callback) = 0;
    virtual RegID requestActionAsync(
            OpID opID,
            const CSMsgContentBasePtr& msgContent,
            CSMessageContentHandlerCallback callback
            ) = 0;

    virtual CSMsgContentBasePtr getStatus(
            OpID propertyID,
            unsigned long maxWaitTimeMs
            ) = 0;
    virtual CSMsgContentBasePtr requestAction(
            OpID opID,
            const CSMsgContentBasePtr& msgContent,
            unsigned long maxWaitTimeMs
            ) = 0;

    virtual void abortAction(const RegID& regID) = 0;

    virtual Availability serviceStatus() const = 0;

    virtual void addServiceStatusObserver(
            std::weak_ptr<ServiceStatusObserverInterface> pServiceStatusObserver
            ) = 0;
    virtual void removeServiceStatusObserver(
            const std::weak_ptr<ServiceStatusObserverInterface>& pServiceStatusObserver
            ) = 0;
    virtual ~ServiceRequesterInterface() = default;
};

}
}
