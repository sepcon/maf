#pragma once

#include "ClientInterface.h"
#include "ServerInterface.h"
#include "thaf/patterns/Patterns.h"
#include "IAMessageTrait.h"
#include "ClientBase.h"
#include "ServerBase.h"

namespace thaf {
namespace messaging {


class IAMessageRouter : public ClientBase, public ServerBase, public pattern::SingletonObject<IAMessageRouter>
{
public:
    IAMessageRouter(Invisible){}
    void init();
    void deinit();
    bool registerServiceRequester(const std::shared_ptr<ServiceRequesterInterface>& requester)  override;
    DataTransmissionErrorCode sendMessageToClient(const CSMessagePtr& msg, const Address& addr = Address::INVALID_ADDRESS) override;
    DataTransmissionErrorCode sendMessageToServer(const CSMessagePtr& msg) override;
    void notifyServiceStatusToClient(ServiceID sid, Availability oldStatus, Availability newStatus) override;
};

}
}
