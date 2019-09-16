#pragma once

#include "ServiceStubInterface.h"

namespace maf {
namespace messaging {
class ServerInterface;
class ServiceStubBaseImpl;

class ServiceStubBase : public ServiceStubInterface
{
public:
    ServiceStubBase(ServiceID sid, class ServerInterface* server, ServiceStubHandlerInterface* stubHandler = nullptr);
    ~ServiceStubBase() override;
    void setStubHandler(ServiceStubHandlerInterface *stubHandler) override;
    bool replyToRequest(const CSMessagePtr &csMsg, bool hasDone = true) override;
    bool sendStatusUpdate(const CSMessagePtr& csMsg) override;

private:
    bool onIncomingMessage(const CSMessagePtr& msg) override;
    ServiceStubBaseImpl* _pImpl;
};

}// messaging
}// maf
