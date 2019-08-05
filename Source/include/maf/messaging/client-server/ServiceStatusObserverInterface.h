#pragma once

#include "CSStatus.h"
#include "CSMessage.h"

namespace maf {
namespace messaging {

class ServiceStatusObserverInterface
{
public:
    virtual ~ServiceStatusObserverInterface() = default;
    virtual void onServerStatusChanged(Availability oldStatus, Availability newStatus) = 0;
    virtual void onServiceStatusChanged(ServiceID sid, Availability oldStatus, Availability newStatus) = 0;
};
}
}
