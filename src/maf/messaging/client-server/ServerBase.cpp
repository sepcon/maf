#include "ServerBase.h"
#include "ServiceProvider.h"
#include <maf/logging/Logger.h>
#include <maf/utils/containers/Map.h>

namespace maf {

namespace messaging {

bool ServerBase::registerServiceProvider(const ServiceProviderIFPtr &provider) {
  if (provider) {
    std::unique_lock lock(providers_);
    auto [itInsertedPos, inserted] =
        providers_->try_emplace(provider->serviceID(), provider);

    if (inserted) {
      lock.unlock();
      MAF_LOGGER_INFO("New Service provider was successfully registered, "
                      "service id = ",
                      provider->serviceID());
      notifyServiceStatusToClient(provider->serviceID(),
                                  Availability::Unavailable,
                                  Availability::Available);
      return true;
    }
  }

  return false;
}

bool ServerBase::unregisterServiceProvider(
    const ServiceProviderIFPtr &provider) {
  return unregisterServiceProvider(provider->serviceID());
}

bool ServerBase::unregisterServiceProvider(const ServiceID &sid) {
  if (providers_.atomic()->erase(sid) != 0) {
    notifyServiceStatusToClient(sid, Availability::Available,
                                Availability::Unavailable);
    return true;
  } else {
    return false;
  }
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

bool ServerBase::deinit() {
  auto providers = *(providers_.atomic());
  providers_.atomic()->clear();
  return true;
}

} // namespace messaging
} // namespace maf
