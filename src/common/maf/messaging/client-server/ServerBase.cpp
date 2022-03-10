#include "ServerBase.h"

#include <maf/logging/Logger.h>
#include <maf/utils/containers/Map.h>

#include "ServiceProvider.h"

namespace maf {
namespace messaging {

ServiceProviderIFPtr ServerBase::getServiceProvider(const ServiceID &sid) {
  std::lock_guard lock(providers_);
  auto &provider = (*providers_)[sid];
  if (!provider) {
    provider = std::make_shared<ServiceProvider>(sid, weak_from_this());
  }
  return provider;
}

bool ServerBase::hasServiceProvider(const ServiceID &sid) {
  return providers_.atomic()->count(sid) != 0;
}

bool ServerBase::onIncomingMessage(const CSMessagePtr &csMsg) {
  std::unique_lock lock(providers_);
  if (auto provider = util::get(*providers_, csMsg->serviceID())) {
    lock.unlock();
    return provider->onIncomingMessage(csMsg);
  } else {
    return false;
  }
}

bool ServerBase::init(const Address &) { return true; }

void ServerBase::deinit() {
  auto providers = std::move(*providers_.atomic());

  for (auto &[_, provider] : providers) {
    provider->stopServing();
  }
  for (auto &[_, provider] : providers) {
    provider->deinit();
  }
}

void ServerBase::onServiceStatusChanged(const ServiceID &sid,
                                        Availability oldStatus,
                                        Availability newStatus) {
  if (oldStatus != newStatus) {
    if (hasServiceProvider(sid)) {
      notifyServiceStatusToClient(sid, oldStatus, newStatus);
    }
  }
}

}  // namespace messaging
}  // namespace maf
