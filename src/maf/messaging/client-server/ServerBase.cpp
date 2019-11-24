#include <maf/messaging/client-server/ServerBase.h>
#include <maf/messaging/client-server/ServiceProvider.h>
#include <maf/utils/containers/Map.h>
#include <maf/logging/Logger.h>


namespace maf { using logging::Logger;
namespace messaging {


bool ServerBase::registerServiceProvider(
        const ServiceProviderInterfacePtr& provider
        )
{
    std::lock_guard lock(_providers);
    if(provider)
    {
        auto [itInsertedPos, successInserted] =
                _providers->try_emplace(provider->serviceID(), provider);
        if(successInserted)
        {
            Logger::info("New Service provider was successfully registered, "
                         "service id = " ,
                         provider->serviceID()
                         );
            notifyServiceStatusToClient(provider->serviceID(),
                                        Availability::Unavailable,
                                        Availability::Available
                                        );
            return true;
        }
    }

    return false;
}

bool ServerBase::unregisterServiceProvider(
        const ServiceProviderInterfacePtr& provider
        )
{
    return unregisterServiceProvider(provider->serviceID());
}

bool ServerBase::unregisterServiceProvider(ServiceID sid)
{ 
    if(_providers.atomic()->erase(sid) != 0)
    {
        notifyServiceStatusToClient(
                    sid,
                    Availability::Available,
                    Availability::Unavailable
                    );
        return true;
    }
    else
    {
        return false;
    }
}

bool ServerBase::hasServiceProvider(ServiceID sid)
{
    return _providers.atomic()->count(sid) != 0;
}

bool ServerBase::onIncomingMessage(const CSMessagePtr &csMsg)
{
    std::unique_lock lock(_providers);
    if(auto provider = util::get(*_providers, csMsg->serviceID()))
    {
        lock.unlock();
        return provider->onIncomingMessage(csMsg);
    }
    else
    {
        return false;
    }
}

bool ServerBase::init(const Address &)
{
    return true;
}

bool ServerBase::deinit()
{
    auto providers = *(_providers.atomic());
    _providers.atomic()->clear();
    return true;
}

}
}
