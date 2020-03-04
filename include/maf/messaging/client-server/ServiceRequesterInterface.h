#pragma once

#include "RegID.h"
#include "ServiceStatusObserverInterface.h"
#include "ServiceMessageReceiver.h"
#include "internal/CSShared.h"
#include <chrono>

namespace maf {
namespace messaging {

using RequestTimeoutMs = std::chrono::milliseconds;

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

    virtual ActionCallStatus unregisterBroadcast(const RegID& regID) = 0;
    virtual ActionCallStatus unregisterBroadcastAll(const OpID& propertyID) = 0;

    virtual RegID sendRequestAsync(
        const OpID& opID,
        const CSMsgContentBasePtr& msgContent,
        CSMessageContentHandlerCallback callback,
        ActionCallStatus* callStatus
        ) = 0;

    virtual CSMsgContentBasePtr getStatus(
        const OpID& propertyID,
        RequestTimeoutMs timeout,
        ActionCallStatus* callStatus
        ) = 0;
    virtual CSMsgContentBasePtr sendRequest(
        const OpID& opID,
        const CSMsgContentBasePtr& msgContent,
        RequestTimeoutMs timeout,
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
