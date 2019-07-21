#include "thaf/messaging/client-server/IAServiceProxy.h"
#include "thaf/messaging/client-server/IAMessageRouter.h"

namespace thaf {
namespace messaging {

//IAServiceProxy::IAServiceProxy() : QueueingServiceProxy<IAMessageTrait> (&IAMessageRouter::instance())
//{
//}

//IAServiceProxy::~IAServiceProxy()
//{
//    IAMessageRouter::instance().unregisterServiceRequester(serviceID());
//}

//std::shared_ptr<IAServiceProxy> IAServiceProxy::createProxy(ServiceID sid)
//{
//    auto serviceRequester = IAMessageRouter::instance().getServiceRequeser(sid);
//    if(!serviceRequester)
//    {
//        serviceRequester = std::make_shared<IAServiceProxy>();
//        IAMessageRouter::instance().registerServiceRequester(serviceRequester);
//    }
//    auto proxy = std::static_pointer_cast<IAServiceProxy>(serviceRequester);
//    proxy->addInterestedComponent(Component::getComponentRef());

//    return proxy;
//}


}
}
