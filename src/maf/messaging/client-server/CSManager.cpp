#include "ClientFactory.h"
#include "ServerFactory.h"
#include "ServiceProvider.h"
#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/CSManager.h>
#include <maf/threading/Lockable.h>
#include <map>
#include <vector>

namespace maf {
namespace messaging {

struct CSManagerImpl {
  threading::Lockable<
      std::map<std::shared_ptr<ServerIF>, std::vector<ServiceProviderIFPtr>>>
      serviceProviders_;

  std::shared_ptr<ServiceRequesterIF>
  getServiceRequester(const ConnectionType &conntype, const Address &serverAddr,
                      const ServiceID &sid) {
    if (auto client =
            ClientFactory::instance().getClient(conntype, serverAddr)) {
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
                                          const ServiceID &sid) {
    std::shared_ptr<ServiceProviderIF> provider;

    if (auto server =
            ServerFactory::instance().getServer(conntype, serverAddr)) {
      std::lock_guard lock(serviceProviders_);
      // if already had provider on found server
      if (auto itProviders = serviceProviders_->find(server);
          itProviders != serviceProviders_->end()) {
        for (auto &p : itProviders->second) {
          if (p->serviceID() == sid) {
            provider = p;
            break;
          }
        }
      }

      if (!provider) {
        provider = std::make_shared<ServiceProvider>(sid, std::move(server));
        (*serviceProviders_)[server].push_back(provider);
      }
    } else {
      MAF_LOGGER_FATAL("Could not found server with given connection type:"
                       "[",
                       conntype, "] and addr: ", serverAddr.dump(-1));
    }

    return provider;
  }
};

CSManager::CSManager() : pImpl_{std::make_unique<CSManagerImpl>()} {}

CSManager::~CSManager() = default;

CSManager &CSManager::instance() noexcept {
  static CSManager _instance;
  return _instance;
}

std::shared_ptr<ServiceRequesterIF>
CSManager::getServiceRequester(const ConnectionType &conntype,
                               const Address &serverAddr,
                               const ServiceID &sid) noexcept {
  return pImpl_->getServiceRequester(conntype, serverAddr, sid);
}

std::shared_ptr<ServiceProviderIF>
CSManager::getServiceProvider(const ConnectionType &conntype,
                              const Address &serverAddr, const ServiceID &sid) noexcept {
  return pImpl_->getServiceProvider(conntype, serverAddr, sid);
}

} // namespace messaging
} // namespace maf
