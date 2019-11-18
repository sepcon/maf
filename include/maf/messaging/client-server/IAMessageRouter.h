#pragma once

#include "ClientInterface.h"
#include "ServerInterface.h"
#include <maf/patterns/Patterns.h>
#include "DefaultMessageTrait.h"
#include "ClientBase.h"
#include "ServerBase.h"

namespace maf {
namespace messaging {


class IAMessageRouter :
    public ClientBase,
    public ServerBase,
    pattern::Unasignable
{
public:
    static std::shared_ptr<IAMessageRouter> instance();
    bool deinit() override;
    ActionCallStatus sendMessageToClient(const CSMessagePtr& msg, const Address& addr = Address::INVALID_ADDRESS) override;
    ActionCallStatus sendMessageToServer(const CSMessagePtr& msg) override;
    void notifyServiceStatusToClient(ServiceID sid, Availability oldStatus, Availability newStatus) override;
};

}
}
