#pragma once

#include <maf/messaging/Processor.h>

#include <cassert>

#include "ServiceStatusObserverIF.h"

namespace maf {
namespace messaging {

class AsyncServiceStatusObserver : public ServiceStatusObserverIF {
  using CompWPtr = std::weak_ptr<Processor>;
  using CompPtr = std::shared_ptr<Processor>;

 public:
  AsyncServiceStatusObserver(const CompPtr& comp) : _wpcomp{comp} {
    assert(comp && "Processor must not be nullptr");
  }

 private:
  void onServiceStatusChanged(const ServiceID &sid, Availability oldStatus,
                              Availability newStatus) override;
  void notifyServiceStatusChange(const ServiceID &sid, Availability oldStatus,
                                 Availability newStatus);
  CompWPtr _wpcomp;
};

inline std::shared_ptr<ServiceStatusObserverIF> asyncServiceStatusObserver(
    std::shared_ptr<Processor> comp) {
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
    const ServiceID &sid, Availability oldStatus, Availability newStatus) {
  notifyServiceStatusChange(sid, oldStatus, newStatus);
}

inline void AsyncServiceStatusObserver::notifyServiceStatusChange(
    const ServiceID &sid, Availability oldStatus, Availability newStatus) {
  if (auto comp = _wpcomp.lock()) {
    comp->post<ServiceStatusMsg>(sid, oldStatus, newStatus);
  }
}

}  // namespace messaging
}  // namespace maf
