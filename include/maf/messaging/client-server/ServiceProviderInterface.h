#pragma once

#include "ServiceProviderShared.h"
#include "ServiceMessageReceiver.h"

namespace maf {
namespace messaging {

class ServiceProviderInterface : public ServiceMessageReceiver
{
public:
    virtual ~ServiceProviderInterface() = default;
    virtual void startServing() = 0;
    virtual void stopServing() = 0;
    virtual bool registerRequestHandler(
        OpID opID,
        RequestHandlerFunction handlerFunction
        ) = 0;

    virtual bool unregisterRequestHandler( OpID opID ) = 0;

    virtual ActionCallStatus respondToRequest(
        const CSMessagePtr &csMsg
        ) = 0;
    virtual ActionCallStatus setStatus(
        OpID propertyID,
        const CSMsgContentBasePtr& property
        ) = 0;

    virtual CSMsgContentBasePtr getStatus(OpID propertyID) = 0;
};

}
}
