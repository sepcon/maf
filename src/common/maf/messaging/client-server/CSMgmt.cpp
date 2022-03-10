#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/CSMgmt.h>

#include "ClientFactory.h"
#include "ServerFactory.h"
#include "SingleThreadPool.h"

namespace maf {
namespace messaging {
namespace csmgmt {

static ClientFactory *clientFactory() {
  static auto factory = new ClientFactory;
  return factory;
}
static ServerFactory *serverFactory() {
  static auto factory = new ServerFactory;
  return factory;
}

std::shared_ptr<ServiceRequesterIF> getServiceRequester(
    const ConnectionType &conntype, const Address &serverAddr,
    const ServiceID &sid) noexcept {
  if (auto client = clientFactory()->getClient(conntype, serverAddr)) {
    if (auto requester = client->getServiceRequester(sid)) {
      return requester;
    } else {
      MAF_LOGGER_ERROR("Could not get requester for service [", sid, "]");
    }
  } else {
    MAF_LOGGER_FATAL("Failed to create client with connection type: [",
                     conntype, "], and address: [", serverAddr.dump(-1), "]");
  }
  return {};
}

ServiceProviderIFPtr getServiceProvider(const ConnectionType &conntype,
                                        const Address &serverAddr,
                                        const ServiceID &sid) noexcept {
  if (auto server = serverFactory()->getServer(conntype, serverAddr)) {
    // if already had provider on found server
    return server->getServiceProvider(sid);
  }
  return {};
}

void shutdownAllClients() { clientFactory()->close(); }

void shutdownAll() {
  shutdownAllServers();
  shutdownAllClients();
}

void shutdownAllServers() { serverFactory()->close(); }

}  // namespace csmgmt
}  // namespace messaging
}  // namespace maf
