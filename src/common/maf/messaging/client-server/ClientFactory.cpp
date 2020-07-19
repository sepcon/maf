#include "ClientFactory.h"

#include <maf/logging/Logger.h>
#include <maf/utils/containers/Map2D.h>

#include "ipc/LocalIPCClient.h"
#include "itc/Client.h"

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
        client->stop();
      }
      for (auto &[addr, client] : clients) {
        client->deinit();
      }
    }
  }

  std::shared_ptr<ClientIF> makeClient(const ConnectionType &connectionType) {
    if (connectionType == "itc.messaging.maf") {
      return itc::Client::instance();
    } else if (connectionType == "local.ipc.messaging.maf") {
      return std::make_shared<ipc::local::LocalIPCClient>();
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
    if ((client = util::find(*_clientMap, connectionType, addr)); !client) {
      if ((client = makeClient(connectionType)) && client->init(addr)) {
        (*_clientMap)[connectionType].emplace(addr, client);
        client->start();
      } else {
        client.reset();
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

std::shared_ptr<ClientIF> ClientFactory::getClient(
    const ConnectionType &connectionType, const Address &addr) {
  return _pImpl->getClient(connectionType, addr);
}

}  // namespace messaging
}  // namespace maf
