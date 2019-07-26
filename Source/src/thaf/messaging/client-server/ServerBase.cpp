#include "thaf/messaging/client-server/ServerBase.h"
#include "thaf/messaging/client-server/interfaces/ServiceProviderInterface.h"
#include "thaf/utils/debugging/Debug.h"

namespace thaf {
namespace messaging {


bool ServerBase::registerServiceProvider(const std::shared_ptr<ServiceProviderInterface> &provider)
{
    auto newProvider = addIfNew(_providers, provider);
    if(newProvider)
    {
        thafInfo("New Service provider was successfully registered, service id = " << provider->serviceID());
        notifyServiceStatusToClient(provider->serviceID(), Availability::Unavailable, Availability::Available);
        return true;
    }
    else
    {
        return false;
    }
}

bool ServerBase::unregisterServiceProvider(const std::shared_ptr<ServiceProviderInterface> &provider)
{
    return unregisterServiceProvider(provider->serviceID());
}

bool ServerBase::unregisterServiceProvider(ServiceID sid)
{
    auto providerRemoved = removeByID(_providers, sid);
    if(providerRemoved)
    {
        notifyServiceStatusToClient(sid, Availability::Available, Availability::Unavailable);
        return true;
    }
    else
    {
        return false;
    }
}

bool ServerBase::hasServiceProvider(ServiceID sid)
{
    return hasItemWithID(_providers, sid);
}

bool ServerBase::onIncomingMessage(const CSMessagePtr &csMsg)
{
    auto provider = findByID(_providers, csMsg->serviceID());
    if(provider)
    {
        provider->onIncomingMessage(csMsg);
        return true;
    }
    else
    {
        return false;
    }
}

std::shared_ptr<ServiceProviderInterface> ServerBase::getServiceProvider(ServiceID sid)
{
    return findByID(_providers, sid);
}

}
}
