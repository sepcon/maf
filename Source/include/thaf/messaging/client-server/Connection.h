#pragma once

#include "thaf/messaging/BasicMessages.h"
#include "thaf/messaging/client-server/interfaces/CSStatus.h"

namespace thaf {
namespace messaging {

struct ServiceStatusMsg : public ExternalMessage
{
    ServiceStatusMsg(Availability old_, Availability new_):
        oldStatus(old_), newStatus(new_){}

    Availability oldStatus;
    Availability newStatus;
};

}// messaging
}// thaf
