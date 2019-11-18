#include <maf/messaging/client-server/ServerFactory.h>
#include <maf/messaging/client-server/IAMessageRouter.h>
#include <maf/messaging/client-server/ipc/LocalIPCServer.h>
#include <maf/utils/containers/Map2D.h>
#include <maf/logging/Logger.h>


namespace maf {
namespace messaging {
using logging::Logger;

class ServerFactoryImpl
{
    threading::Lockable<
        util::Map2D<ConnectionType, Address, std::shared_ptr<ServerInterface>>
                        > _serverMap;

public:
    ~ServerFactoryImpl()
    {
        std::lock_guard lock(_serverMap);
        for(auto& [contype, servers] : *_serverMap )
        {
            for(auto& [addr, server] : servers)
            {
                server->deinit();
            }
        }
    }

    std::shared_ptr<ServerInterface> makeServer(const ConnectionType& connectionType)
    {
        if(connectionType == "app_internal")
        {
            return IAMessageRouter::instance();
        }
        else if(connectionType == "local_ipc")
        {
            return std::make_shared<ipc::LocalIPCServer>();
        }
        else
        {
            Logger::error("Request creating with non-exist connection type [", connectionType, "]");
            return {};
        }
    }

    std::shared_ptr<ServerInterface> getServer(const ConnectionType& connectionType, const Address& addr)
    {
        std::shared_ptr<ServerInterface> server;
        std::lock_guard lock(_serverMap);
        if((server = util::find(*_serverMap, connectionType, addr));
            server == nullptr)
        {
            if((server = makeServer(connectionType)))
            {
                (*_serverMap)[connectionType].emplace(addr, server);
                server->init(addr);
            }
            else
            {
                Logger::fatal("Could not get and create server of type ", connectionType, " and addr = ", addr.dump(-1));
            }
        }
        return server;
    }
};

ServerFactory::ServerFactory(Invisible)
{
    _pImpl = std::make_unique<ServerFactoryImpl>();
}

ServerFactory::~ServerFactory() = default;

std::shared_ptr<ServerInterface> ServerFactory::getServer(const ConnectionType &connectionType, const Address &addr)
{
    return _pImpl->getServer(connectionType, addr);
}

} // messaging
} // maf
