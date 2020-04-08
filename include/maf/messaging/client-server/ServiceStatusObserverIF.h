#pragma once

#include "CSStatus.h"
#include "CSTypes.h"

namespace maf {
namespace messaging {

class ServiceStatusObserverIF {
public:
  virtual void onServiceStatusChanged(const ServiceID &sid,
                                      Availability oldStatus,
                                      Availability newStatus) noexcept = 0;
};
} // namespace messaging
} // namespace maf
