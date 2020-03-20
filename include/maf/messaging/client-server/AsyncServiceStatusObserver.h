#ifndef ASYNCSERVICESTATUSOBSERVER_H
#define ASYNCSERVICESTATUSOBSERVER_H

#include "ServiceStatusObserverIF.h"
#include <cassert>
#include <maf/messaging/Component.h>

namespace maf {
namespace messaging {

class AsyncServiceStatusObserver : public ServiceStatusObserverIF {
  using CompWPtr = std::weak_ptr<Component>;
  using CompPtr = std::shared_ptr<Component>;

public:
  AsyncServiceStatusObserver(CompPtr comp) : _wpcomp{comp} {
    assert(comp && "Component must not be nullptr");
  }

  void onServiceStatusChanged(const ServiceID &sid, Availability oldStatus,
                              Availability newStatus) noexcept override;

private:
  void notifyServiceStatusChange(const ServiceID &sid, Availability oldStatus,
                                 Availability newStatus);
  CompWPtr _wpcomp;
};

inline std::shared_ptr<ServiceStatusObserverIF>
asyncServiceStatusObserver(std::shared_ptr<Component> comp) {
  return std::shared_ptr<AsyncServiceStatusObserver>{
      new AsyncServiceStatusObserver{std::move(comp)}};
}
struct ServiceStatusMsg {
  ServiceStatusMsg(ServiceID sid_, Availability old_, Availability new_)
      : serviceID(std::move(sid_)), oldStatus(old_), newStatus(new_) {}

  ServiceID serviceID;
  Availability oldStatus;
  Availability newStatus;
};

inline void AsyncServiceStatusObserver::onServiceStatusChanged(
    const ServiceID &sid, Availability oldStatus,
    Availability newStatus) noexcept {
  notifyServiceStatusChange(sid, oldStatus, newStatus);
}

inline void AsyncServiceStatusObserver::notifyServiceStatusChange(
    const ServiceID &sid, Availability oldStatus, Availability newStatus) {
  if (auto comp = _wpcomp.lock()) {
    comp->post<ServiceStatusMsg>(sid, oldStatus, newStatus);
  }
}

} // namespace messaging
} // namespace maf

#endif // ASYNCSERVICESTATUSOBSERVER_H
