#pragma once

#include <maf/messaging/client-server/ServiceRequesterInterface.h>
#include "CSDefines.h"

namespace maf {
namespace messaging {

class ClientInterface;
struct ServiceRequesterImpl;

class ServiceRequester : public ServiceRequesterInterface
{
    std::unique_ptr<ServiceRequesterImpl> _pImpl;
public:
    ServiceRequester(const ServiceID& sid, std::weak_ptr<ClientInterface> client);
    ~ServiceRequester() override;

    Availability serviceStatus() const override;

    RegID registerStatus(
        const OpID& propertyID,
        CSMessageContentHandlerCallback callback,
        ActionCallStatus* callStatus
        ) override;

    RegID registerSignal(
        const OpID& propertyID,
        CSMessageContentHandlerCallback callback,
        ActionCallStatus* callStatus
        ) override;

    ActionCallStatus unregisterBroadcast(const RegID &regID) override;
    ActionCallStatus unregisterBroadcastAll(const OpID& propertyID) override;

    RegID sendRequestAsync(
        const OpID& opID,
        const CSMsgContentBasePtr& msgContent,
        CSMessageContentHandlerCallback callback,
        ActionCallStatus* callStatus
        ) override;

    CSMsgContentBasePtr getStatus(
        const OpID& propertyID,
        unsigned long maxWaitTimeMs,
        ActionCallStatus* callStatus
        ) override;

    CSMsgContentBasePtr sendRequest(
        const OpID& opID,
        const CSMsgContentBasePtr& msgContent,
        unsigned long maxWaitTimeMs,
        ActionCallStatus* callStatus
        ) override;

    void abortAction(
        const RegID& regID,
        ActionCallStatus* callStatus
        ) override;

    void addServiceStatusObserver(
        std::weak_ptr<ServiceStatusObserverInterface> serviceStatusObserver
        ) override;

    void removeServiceStatusObserver(
        const std::weak_ptr<ServiceStatusObserverInterface>& serviceStatusObserver
        ) override;

private:
    bool onIncomingMessage(const CSMessagePtr& csMsg) override;
    void onServerStatusChanged(
        Availability oldStatus,
        Availability newStatus
        ) override;
    void onServiceStatusChanged(
        const ServiceID& sid,
        Availability oldStatus,
        Availability newStatus
        ) override;
};

}// messaging
}// maf

