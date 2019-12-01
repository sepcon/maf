#pragma once

#include "ClientBase.h"
#include "ServerBase.h"
#include <maf/patterns/Patterns.h>

namespace maf {
namespace messaging {


// TODO: Multiple inheritance here makes enable_shared_from_this object will not
// be available from both ClientBase and ServerBase. Just one of them has object
// weak_ptr ref available, then might cause the problem later
class IAMessageRouter :
    public ClientBase,
    public ServerBase,
    pattern::Unasignable
{
public:
    static std::shared_ptr<IAMessageRouter> instance();
    bool deinit() override;
    ActionCallStatus sendMessageToClient(const CSMessagePtr& msg, const Address& addr = {}) override;
    ActionCallStatus sendMessageToServer(const CSMessagePtr& msg) override;
    void notifyServiceStatusToClient(const ServiceID& sid, Availability oldStatus, Availability newStatus) override;
};

}
}
