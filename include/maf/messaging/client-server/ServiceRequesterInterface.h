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
        const OpID& propertyID,
        CSMessageContentHandlerCallback callback,
        ActionCallStatus* callStatus
        ) = 0;

    virtual RegID registerSignal(
        const OpID& propertyID,
        CSMessageContentHandlerCallback callback,
        ActionCallStatus* callStatus
        ) = 0;

    virtual ActionCallStatus unregisterStatus(const RegID& regID) = 0;
    virtual ActionCallStatus unregisterStatusAll(const OpID& propertyID) = 0;

    virtual RegID sendRequestAsync(
        const OpID& opID,
        const CSMsgContentBasePtr& msgContent,
        CSMessageContentHandlerCallback callback,
        ActionCallStatus* callStatus
        ) = 0;

    virtual CSMsgContentBasePtr getStatus(
        const OpID& propertyID,
        unsigned long maxWaitTimeMs,
        ActionCallStatus* callStatus
        ) = 0;
    virtual CSMsgContentBasePtr sendRequest(
        const OpID& opID,
        const CSMsgContentBasePtr& msgContent,
        unsigned long maxWaitTimeMs,
        ActionCallStatus* callStatus
        ) = 0;

    virtual void abortAction(
        const RegID& regID,
        ActionCallStatus* callStatus
        ) = 0;

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
