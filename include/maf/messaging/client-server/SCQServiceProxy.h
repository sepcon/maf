#pragma once

#include "QueueingServiceProxy.h"
#include <maf/patterns/Patterns.h>
#include <maf/utils/cppextension/maf.mc.h>

namespace maf {
namespace messaging {

template <class MessageTrait, class SingletonClient,
         std::enable_if_t<std::is_base_of_v<pattern::SingletonObject<SingletonClient>, SingletonClient>, bool> = true>
class SCQServiceProxy final : public QueueingServiceProxy<MessageTrait>
{
    using _MyBase = QueueingServiceProxy<MessageTrait>;
    using _MyClient = SingletonClient;
public:

    static std::shared_ptr<SCQServiceProxy> createProxy(ServiceID sid) maf_throws(std::runtime_error)
    {
        static std::mutex creatingMutex;
        std::lock_guard<std::mutex> lock(creatingMutex);
        auto serviceRequester = SingletonClient::instance().getServiceRequester(sid);

        if(serviceRequester && typeid (*serviceRequester) != typeid(SCQServiceProxy))
        {
            mafErr("Already had different Proxy type[" << typeid(serviceRequester.get()).name() << "] register to this service id [" << sid << "]!");
            throw std::runtime_error("ClientBase::createProxy -> mismatch between existing Proxy with requested one!");
        }
        else if(!serviceRequester)
        {
            serviceRequester.reset(new SCQServiceProxy(sid)); // the ClientBase class must be friend of Proxy class
            SingletonClient::instance().registerServiceRequester(serviceRequester);
        }

        auto proxy = std::static_pointer_cast<SCQServiceProxy>(serviceRequester);
        proxy->addInterestedComponent(Component::getComponentRef());
        return proxy;
    }

protected:
    SCQServiceProxy(ServiceID sid) : _MyBase(sid, &(_MyClient::instance())){}
};

}
}
