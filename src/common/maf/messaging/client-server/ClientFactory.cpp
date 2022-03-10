#include "ClientFactory.h"

#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/ipc/local/ConnectionType.h>
#include <maf/messaging/client-server/itc/ConnectionType.h>
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
  ~ClientFactoryImpl() { close(); }

  std::shared_ptr<ClientIF> makeClient(const ConnectionType &connectionType) {
    if (connectionType == itc::connection_type) {
      return itc::makeClient();
    } else if (connectionType == ipc::local::connection_type) {
      return ipc::local::makeClient();
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
  void close() {
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
};

ClientFactory::ClientFactory()
    : _pImpl{std::make_unique<ClientFactoryImpl>()} {}

ClientFactory::~ClientFactory() = default;

std::shared_ptr<ClientIF> ClientFactory::getClient(
    const ConnectionType &connectionType, const Address &addr) {
  return _pImpl->getClient(connectionType, addr);
}

void ClientFactory::close() { _pImpl->close(); }
}  // namespace messaging
}  // namespace maf
