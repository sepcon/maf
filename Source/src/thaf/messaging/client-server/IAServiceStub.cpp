#include "thaf/messaging/client-server/IAServiceStub.h"
#include "thaf/messaging/client-server/IAMessageRouter.h"

namespace thaf {
namespace messaging {

//IAServiceStub::IAServiceStub() : QueueingServiceStub<IAMessageTrait> (&IAMessageRouter::instance())
//{
//}

//std::shared_ptr<IAServiceStub> IAServiceStub::createStub(ServiceID sid)
//{
//    auto serviceProvider = IAMessageRouter::instance().getServiceProvider(sid);
//    if(!serviceProvider)
//    {
//        serviceProvider = std::make_shared<IAServiceStub>();
//        IAMessageRouter::instance().registerServiceProvider(serviceProvider);
//    }
//    auto stub = std::static_pointer_cast<IAServiceStub>(serviceProvider);

//    return stub;
//}


}
}
