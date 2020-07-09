#include "ServerFactory.h"
#include "ipc/LocalIPCServer.h"
#include "itc/Server.h"
#include <maf/logging/Logger.h>
#include <maf/utils/containers/Map2D.h>

namespace maf {
namespace messaging {

class ServerFactoryImpl {
  threading::Lockable<
      util::Map2D<ConnectionType, Address, std::shared_ptr<ServerIF>>>
      serverMap_;

public:
  ~ServerFactoryImpl() {
    std::lock_guard lock(serverMap_);
    for (auto &[contype, servers] : *serverMap_) {
      for (auto &[addr, server] : servers) {
        server->stop();
      }
      for (auto &[addr, server] : servers) {
        server->deinit();
      }
    }
  }

  std::shared_ptr<ServerIF> makeServer(const ConnectionType &connectionType) {
    if (connectionType == "itc.messaging.maf") {
      return itc::Server::instance();
    } else if (connectionType == "local.ipc.messaging.maf") {
      return std::make_shared<ipc::LocalIPCServer>();
    } else {
      MAF_LOGGER_ERROR("Request creating with non-exist connection type [",
                       connectionType, "]");
      return {};
    }
  }

  std::shared_ptr<ServerIF> getServer(const ConnectionType &connectionType,
                                      const Address &addr) {
    std::shared_ptr<ServerIF> server;
    std::lock_guard lock(serverMap_);
    if ((server = util::find(*serverMap_, connectionType, addr));
        server == nullptr) {
      if ((server = makeServer(connectionType))) {
        if (server->init(addr)) {
          (*serverMap_)[connectionType].emplace(addr, server);
          server->start();
        } else {
          server.reset();
        }
      } else {
        MAF_LOGGER_FATAL("Could not get and create server of type ",
                         connectionType, " and addr = ", addr.dump(-1));
      }
    }
    return server;
  }
};

ServerFactory::ServerFactory(Invisible) noexcept
    : pImpl_{std::make_unique<ServerFactoryImpl>()} {}

ServerFactory::~ServerFactory() = default;

std::shared_ptr<ServerIF>
ServerFactory::getServer(const ConnectionType &connectionType,
                         const Address &addr) noexcept {
  return pImpl_->getServer(connectionType, addr);
}

} // namespace messaging
} // namespace maf
