#pragma once

#include "QueueingServiceStub.h"
#include <maf/patterns/Patterns.h>
#include <maf/utils/cppextension/Maf.mc.h>

namespace maf {
namespace messaging {

template <class MessageTrait, class SingletonServer,
         std::enable_if_t<std::is_base_of_v<pattern::SingletonObject<SingletonServer>, SingletonServer>, bool> = true>
class SSQServiceStub final : public QueueingServiceStub<MessageTrait>
{
    using _MyBase = QueueingServiceStub<MessageTrait>;
    using _MyServer = SingletonServer;
public:
    static std::shared_ptr<SSQServiceStub> createStub(ServiceID sid)
    {
        auto serviceProvider = _MyServer::instance().getServiceProvider(sid);
        if(!serviceProvider)
        {
            serviceProvider.reset(new SSQServiceStub(sid));
            if(!_MyServer::instance().registerServiceProvider(serviceProvider))
            {
                //Error: there are more than one component trying to create Stub for one service ID
                std::runtime_error("Stub of service ID " + std::to_string(sid) + "has already taken care by another component!");
            }
        }
        else
        {
            throw std::runtime_error("Stub of service ID " + std::to_string(sid) + " has already been taken care by another component!");
        }
        return std::static_pointer_cast<SSQServiceStub>(serviceProvider);
    }
private:
    SSQServiceStub(ServiceID sid) : _MyBase(sid, &(SingletonServer::instance())){}
};


}
}
