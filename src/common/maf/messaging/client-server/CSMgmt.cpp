#include "ClientFactory.h"
#include "ServerFactory.h"
#include "ServiceProvider.h"
#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/CSMgmt.h>
#include <maf/threading/Lockable.h>
#include <map>
#include <vector>

namespace maf {
namespace messaging {
namespace csmgmt {

std::shared_ptr<ServiceRequesterIF>
getServiceRequester(const ConnectionType &conntype, const Address &serverAddr,
                    const ServiceID &sid) noexcept {
  if (auto client = ClientFactory::instance().getClient(conntype, serverAddr)) {
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
  if (auto server = ServerFactory::instance().getServer(conntype, serverAddr)) {
    // if already had provider on found server
    return server->getServiceProvider(sid);
  }
  return {};
}

} // namespace csmgmt
} // namespace messaging
} // namespace maf
