#pragma once

#include "ServiceRequesterInterface.h"
#include "RegisterDataStructure.h"

namespace thaf {
namespace messaging {

class ServiceProxyInterface : public ServiceRequesterInterface
{
public:
    virtual RegID sendStatusChangeRegister(OpID propertyID, CSMessageHandlerCallback callback) = 0;
    virtual void sendStatusChangeUnregister(RegID regID) = 0;
    virtual void sendStatusChangeUnregisterAll(OpID propertyID) = 0;
    virtual RegID sendRequest(const CSMsgContentPtr& msgContent, CSMessageHandlerCallback callback) = 0;
    virtual void sendAbortRequest(const RegID& regID) = 0;
    virtual void sendAbortSyncRequest(const RegID& regID) = 0;
    virtual bool sendRequestSync (const CSMsgContentPtr& msgContent, CSMessageHandlerCallback callback, unsigned long maxWaitTimeMs = static_cast<unsigned long>(-1)) = 0;
    virtual CSMessagePtr sendRequestSync (const CSMsgContentPtr& msgContent, unsigned long maxWaitTimeMs = static_cast<unsigned long>(-1)) = 0;
};


}
}
