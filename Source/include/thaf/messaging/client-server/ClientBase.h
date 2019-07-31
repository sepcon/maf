#pragma once

#include "ClientInterface.h"
#include "ServiceStatusObserverInterface.h"
#include "ServiceRequesterInterface.h"
#include "internal/CSShared.h"
#include "thaf/utils/cppextension/thaf.mc.h"
#include "thaf/utils/debugging/Debug.h"

namespace thaf {
namespace messaging {

class ClientBase : public ClientInterface
{
public:
    //Dervied class must provide implementation for this method
    DataTransmissionErrorCode sendMessageToServer(const CSMessagePtr& msg) override = 0;
    bool registerServiceRequester(const IServiceRequesterPtr& requester)  override;
    bool unregisterServiceRequester(const IServiceRequesterPtr& requester)  override;
    bool unregisterServiceRequester(ServiceID sid) override;
    void onServerStatusChanged(Availability oldStatus, Availability newStatus) override;
    void onServiceStatusChanged(ServiceID sid, Availability oldStatus, Availability newStatus) override;
    bool hasServiceRequester(ServiceID sid) override;
    IServiceRequesterPtr getServiceRequester(ServiceID sid) override;
    void init();
    void deinit();
    template<class Proxy, std::enable_if_t<std::is_base_of_v<ServiceRequesterInterface, Proxy>, bool> = true>
    std::shared_ptr<Proxy> createProxy(ServiceID sid) thaf_throws(std::runtime_error);

protected:
    bool onIncomingMessage(const CSMessagePtr& msg) override;

    using Requesters = SMList<ServiceRequesterInterface>;
    Requesters _requesters;
};

template<class Proxy, std::enable_if_t<std::is_base_of_v<ServiceRequesterInterface, Proxy>, bool>>
std::shared_ptr<Proxy> ClientBase::createProxy(ServiceID sid)
{

    static std::mutex creatingMutex;
    std::lock_guard lock(creatingMutex);
    auto serviceRequester = getServiceRequester(sid);

    if(serviceRequester && typeid (*serviceRequester) != typeid(Proxy))
    {
        thafErr("Already had different Proxy type[" << typeid(serviceRequester.get()).name() << "] register to this service id [" << sid << "]!");
        throw std::runtime_error("ClientBase::createProxy -> mismatch between existing Proxy with requested one!");
    }
    else if(!serviceRequester)
    {
        serviceRequester.reset(new Proxy(sid, this)); // the ClientBase class must be friend of Proxy class
        registerServiceRequester(serviceRequester);
    }

    return std::static_pointer_cast<Proxy>(serviceRequester);
}

} // messaging
} // thaf
