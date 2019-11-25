#include <maf/messaging/client-server/ClientFactory.h>
#include <maf/messaging/client-server/IAMessageRouter.h>
#include <maf/messaging/client-server/ipc/LocalIPCClient.h>
#include <maf/utils/containers/Map2D.h>

namespace maf {
namespace messaging {

class ClientFactoryImpl
{

public:
    threading::Lockable<
        util::Map2D<ConnectionType, Address, std::shared_ptr<ClientInterface>>
        > _clientMap;

public:
    ~ClientFactoryImpl()
    {
        std::lock_guard lock(_clientMap);
        for(auto& [contype, clients] : *_clientMap )
        {
            for(auto& [addr, client] : clients)
            {
                client->deinit();
            }
        }
    }

    std::shared_ptr<ClientInterface> makeClient(const ConnectionType& connectionType)
    {
        if(connectionType == "app_internal")
        {
            return IAMessageRouter::instance();
        }
        else if(connectionType == "local_ipc")
        {
            return std::make_shared<ipc::LocalIPCClient>();
        }
        else
        {
            Logger::error("Request creating with non-exist connection type [", connectionType, "]");
            return {};
        }
    }

    std::shared_ptr<ClientInterface> getClient(const ConnectionType& connectionType, const Address& addr)
    {
        std::shared_ptr<ClientInterface> client;
        std::lock_guard lock(_clientMap);
        if((client = util::find(*_clientMap, connectionType, addr));
            client == nullptr)
        {
            if((client = makeClient(connectionType)))
            {
                (*_clientMap)[connectionType].emplace(addr, client);
                client->init(addr, 500);
            }
            else
            {
                Logger::fatal("Could not get and create client of type ", connectionType, " and addr = ", addr.dump(-1));
            }
        }
        return client;
    }

};

ClientFactory::ClientFactory(Invisible)
    : _pImpl{ std::make_unique<ClientFactoryImpl>() }
{
}

ClientFactory::~ClientFactory() = default;

std::shared_ptr<ClientInterface> ClientFactory::getClient(const ConnectionType &connectionType, const Address &addr)
{
    return _pImpl->getClient(connectionType, addr);
}


} // messaging
} // maf
