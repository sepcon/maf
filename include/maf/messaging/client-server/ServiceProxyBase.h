#pragma once

#include <maf/messaging/client-server/Connection.h>
#include <maf/messaging/client-server/CSStatus.h>
#include <maf/messaging/client-server/ServiceProxyInterface.h>
#include <maf/messaging/client-server/CSDefines.h>

namespace maf {
namespace messaging {

class ClientInterface;
class ServiceProxyBaseImpl;

class ServiceProxyBase : public ServiceProxyInterface
{
public:
    ServiceProxyBase(ServiceID sid, ClientInterface* client);
    ~ServiceProxyBase() override;

    RegID sendStatusChangeRegister(OpID propertyID, CSMessageHandlerCallback callback) override;
    void sendStatusChangeUnregister(RegID regID) override;
    void sendStatusChangeUnregisterAll(OpID propertyID) override;

    RegID sendRequest
        (
            const CSMsgContentPtr& msgContent,
            CSMessageHandlerCallback callback
            ) override;
    bool sendRequestSync
    (
            const CSMsgContentPtr& msgContent,
            CSMessageHandlerCallback callback,
            unsigned long maxWaitTimeMs = maf_INFINITE_WAIT_PERIOD
            ) override;

    CSMessagePtr sendRequestSync
    (
            const CSMsgContentPtr& msgContent,
            unsigned long maxWaitTimeMs = maf_INFINITE_WAIT_PERIOD
            ) override;

    void sendAbortRequest(const RegID& regID) override;
    void sendAbortSyncRequest(const RegID& regID) override;

protected:
    RegID sendRequest(OpID operationID,
                      const CSMsgContentPtr& msgContent = {},
                      CSMessageHandlerCallback callback = {});
    CSMessagePtr sendRequestSync
        (
            OpID operationID,
            const CSMsgContentPtr& msgContent = {},
            unsigned long maxWaitTimeMs = maf_INFINITE_WAIT_PERIOD
            );
    bool onIncomingMessage(const CSMessagePtr& csMsg) override;
    void onServerStatusChanged(Availability oldStatus, Availability newStatus) override;
    void onServiceStatusChanged(ServiceID sid, Availability oldStatus, Availability newStatus) override;
    ClientInterface* getClient();

    ServiceProxyBaseImpl* _pi = nullptr;
};

}// messaging
}// maf

