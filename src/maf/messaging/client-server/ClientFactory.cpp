#include "ClientFactory.h"
#include "IAMessageRouter.h"
#include "ipc/LocalIPCClient.h"
#include <maf/logging/Logger.h>
#include <maf/utils/containers/Map2D.h>

namespace maf {
namespace messaging {

class ClientFactoryImpl {

public:
  threading::Lockable<
      util::Map2D<ConnectionType, Address, std::shared_ptr<ClientIF>>>
      _clientMap;

public:
  ~ClientFactoryImpl() {
    std::lock_guard lock(_clientMap);
    for (auto &[contype, clients] : *_clientMap) {
      for (auto &[addr, client] : clients) {
        client->deinit();
      }
    }
  }

  std::shared_ptr<ClientIF> makeClient(const ConnectionType &connectionType) {
    if (connectionType == "app_internal") {
      return IAMessageRouter::instance();
    } else if (connectionType == "local_ipc") {
      return std::make_shared<ipc::LocalIPCClient>();
    } else {
      MAF_LOGGER_ERROR("Request creating with non-exist connection type [",
                       connectionType, "]");
      return {};
    }
  }

  std::shared_ptr<ClientIF> getClient(const ConnectionType &connectionType,
                                      const Address &addr) {
    std::shared_ptr<ClientIF> client;
    std::lock_guard lock(_clientMap);
    if ((client = util::find(*_clientMap, connectionType, addr));
        client == nullptr) {
      if ((client = makeClient(connectionType))) {
        (*_clientMap)[connectionType].emplace(addr, client);
        client->init(addr, 500);
      } else {
        MAF_LOGGER_FATAL("Could not get and create client of type [",
                         connectionType, "] and addr = ", addr.dump(-1));
      }
    }
    return client;
  }
};

ClientFactory::ClientFactory(Invisible)
    : _pImpl{std::make_unique<ClientFactoryImpl>()} {}

ClientFactory::~ClientFactory() = default;

std::shared_ptr<ClientIF>
ClientFactory::getClient(const ConnectionType &connectionType,
                         const Address &addr) {
  return _pImpl->getClient(connectionType, addr);
}

} // namespace messaging
} // namespace maf
