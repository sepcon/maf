#pragma once

#include "ServiceProviderInterface.h"
#include "ServiceStubHandlerInterface.h"

namespace maf {
namespace messaging {

class ServiceStubInterface : public ServiceProviderInterface
{
public:
    virtual void setStubHandler(ServiceStubHandlerInterface *stubHandler) = 0;
    virtual bool replyToRequest(const CSMessagePtr &csMsg, bool done) = 0;
    virtual bool sendStatusUpdate(const CSMessagePtr& csMsg) = 0;
};


}
}
