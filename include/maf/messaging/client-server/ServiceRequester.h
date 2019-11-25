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
        CSMessageContentHandlerCallback callback
        ) override;

    RegID registerSignal(
        const OpID& propertyID,
        CSMessageContentHandlerCallback callback) override;

    void unregisterStatus(const RegID &regID) override;
    void unregisterStatusAll(const OpID& propertyID) override;

    RegID getStatusAsync(
        const OpID& propertyID,
        CSMessageContentHandlerCallback callback
        ) override;

    RegID sendRequestAsync
        (
            const OpID& opID,
            const CSMsgContentBasePtr& msgContent,
            CSMessageContentHandlerCallback callback
            ) override;

    CSMsgContentBasePtr getStatus(
        const OpID& propertyID,
        unsigned long maxWaitTimeMs
        ) override;

    CSMsgContentBasePtr sendRequest
        (
            const OpID& opID,
            const CSMsgContentBasePtr& msgContent,
            unsigned long maxWaitTimeMs = maf_INFINITE_WAIT_PERIOD
            ) override;

    void abortAction(const RegID& regID) override;

    void addServiceStatusObserver(
        std::weak_ptr<ServiceStatusObserverInterface> serviceStatusObserver
        ) override;

    void removeServiceStatusObserver(
        const std::weak_ptr<ServiceStatusObserverInterface>& serviceStatusObserver
        ) override;

private:
    bool onIncomingMessage(const CSMessagePtr& csMsg) override;
    void onServerStatusChanged(Availability oldStatus, Availability newStatus) override;
    void onServiceStatusChanged(const ServiceID& sid, Availability oldStatus, Availability newStatus) override;
};

}// messaging
}// maf

