#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/CSMgmt.h>

#include "ClientFactory.h"
#include "SingleThreadPool.h"
#include "ServerFactory.h"

namespace maf {
namespace messaging {
namespace csmgmt {

struct CSInit {
  CSInit() { single_threadpool::init(); }
  ~CSInit() { single_threadpool::deinit(); }
};

static void csinit() { static CSInit _; }

std::shared_ptr<ServiceRequesterIF> getServiceRequester(
    const ConnectionType &conntype, const Address &serverAddr,
    const ServiceID &sid) noexcept {
  csinit();
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
  csinit();
  if (auto server = ServerFactory::instance().getServer(conntype, serverAddr)) {
    // if already had provider on found server
    return server->getServiceProvider(sid);
  }
  return {};
}

}  // namespace csmgmt
}  // namespace messaging
}  // namespace maf
