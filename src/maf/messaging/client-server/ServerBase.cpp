#include <maf/messaging/client-server/ServerBase.h>
#include <maf/messaging/client-server/ServiceProviderInterface.h>
#include <maf/utils/debugging/Debug.h>

namespace maf {
namespace messaging {


bool ServerBase::registerServiceProvider(const IServiceProviderPtr &provider)
{
    auto newProvider = addIfNew(_providers, provider);
    if(newProvider)
    {
        mafInfo("New Service provider was successfully registered, service id = " << provider->serviceID());
        notifyServiceStatusToClient(provider->serviceID(), Availability::Unavailable, Availability::Available);
        return true;
    }
    else
    {
        return false;
    }
}

bool ServerBase::unregisterServiceProvider(const IServiceProviderPtr &provider)
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

IServiceProviderPtr ServerBase::getServiceProvider(ServiceID sid)
{
    return findByID(_providers, sid);
}

void ServerBase::init()
{
// TBD: add if needed
}

void ServerBase::deinit()
{
    auto lock = _providers.a_lock();
    _providers->clear();
}

}
}
