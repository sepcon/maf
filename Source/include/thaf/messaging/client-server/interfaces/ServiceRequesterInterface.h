#pragma once

#include "ServiceStatusObserverInterface.h"
#include "ServiceMessageReceiver.h"

namespace thaf {
namespace messaging {

class ServiceRequesterInterface:
    public ServiceMessageReceiver,
    public ServiceStatusObserverInterface
{
};

}
}
