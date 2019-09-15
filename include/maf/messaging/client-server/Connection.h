#pragma once

#include <maf/messaging/BasicMessages.h>
#include <maf/messaging/client-server/CSStatus.h>
#include <maf/messaging/client-server/CSTypes.h>


namespace maf {
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
}// maf
