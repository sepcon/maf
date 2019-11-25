#include <maf/messaging/client-server/CSManager.h>
#include <maf/messaging/client-server/ClientFactory.h>
#include <maf/messaging/client-server/ServerFactory.h>
#include <maf/messaging/client-server/ServiceProvider.h>
#include <maf/threading/Lockable.h>
#include <maf/logging/Logger.h>
#include <vector>
#include <map>

namespace maf {
namespace messaging {

using logging::Logger;

struct CSManagerImpl
{
    threading::Lockable<
        std::map<
            std::shared_ptr<ServerInterface>,
            std::vector<ServiceProviderInterfacePtr>
            >
        > _serviceProviders;

    std::shared_ptr<ServiceRequesterInterface> getServiceRequester(
        const ConnectionType& conntype,
        const Address& serverAddr,
        const ServiceID& sid
        )
    {
        if(auto client = ClientFactory::instance().getClient(conntype, serverAddr))
        {
            if(auto requester = client->getServiceRequester(sid))
            {
                return requester;
            }
            else
            {
                Logger::error("Could not get requester for service [", sid, "]");
            }
        }
        else
        {
            Logger::fatal("Failed to create client with connection type: [",
                          conntype, "], and address: [", serverAddr.dump(-1), "]"
                          );
        }
        return {};
    }

    ServiceProviderInterfacePtr getServiceProvider(
        const ConnectionType& conntype,
        const Address& serverAddr,
        const ServiceID& sid
        )
    {
        std::shared_ptr<ServiceProviderInterface> provider;

        if(auto server = ServerFactory::instance().getServer(
                conntype,
                serverAddr
                )
            )
        {
            std::lock_guard lock(_serviceProviders);
            //if already had provider on found server
            if(auto itProviders = _serviceProviders->find(server);
                itProviders != _serviceProviders->end())
            {
                for(auto& p : itProviders->second)
                {
                    if(p->serviceID() == sid)
                    {
                        provider = p;
                        break;
                    }
                }
            }

            if(!provider)
            {
                provider = std::make_shared<ServiceProvider>(
                    sid,
                    std::move(server)
                    );
                (*_serviceProviders)[server].push_back(provider);
            }
        }
        else
        {
            Logger::fatal("Could not found server with given connection type:"
                          "[", conntype, "] and addr: ", serverAddr.dump(-1));
        }

        return provider;
    }
};

CSManager::CSManager() :
    _pImpl{std::make_unique<CSManagerImpl>()}
{
}

CSManager::~CSManager() = default;

CSManager &CSManager::instance()
{
    static CSManager _instance;
    return _instance;
}

std::shared_ptr<ServiceRequesterInterface> CSManager::getServiceRequester(
    const ConnectionType& conntype,
    const Address& serverAddr,
    const ServiceID& sid
    )
{
    return _pImpl->getServiceRequester(conntype, serverAddr, sid);
}

std::shared_ptr<ServiceProviderInterface> CSManager::getServiceProvider(
    const ConnectionType& conntype,
    const Address& serverAddr,
    const ServiceID& sid
    )
{
    return _pImpl->getServiceProvider(conntype, serverAddr, sid);
}


} // messaging
} // maf
