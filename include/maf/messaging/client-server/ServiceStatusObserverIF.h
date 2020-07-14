#pragma once

#include <maf/export/MafExport_global.h>

#include "CSStatus.h"
#include "CSTypes.h"

namespace maf {
namespace messaging {

class MAF_EXPORT ServiceStatusObserverIF {
 public:
  virtual ~ServiceStatusObserverIF() = default;
  virtual void onServiceStatusChanged(const ServiceID &sid,
                                      Availability oldStatus,
                                      Availability newStatus) = 0;
};
}  // namespace messaging
}  // namespace maf
