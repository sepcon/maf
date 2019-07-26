#pragma once

#include "thaf/messaging/BasicMessages.h"
#include "thaf/messaging/client-server/interfaces/CSStatus.h"
#include "thaf/messaging/client-server/interfaces/CSTypes.h"


namespace thaf {
namespace messaging {

struct ServiceStatusMsg : public ExternalMessage
{
    ServiceStatusMsg(ServiceID sid_, Availability old_, Availability new_):
        serviceID(sid_), oldStatus(old_), newStatus(new_){}

    ServiceID serviceID;
    Availability oldStatus;
    Availability newStatus;
};

}// messaging
}// thaf
