#pragma once

#include "ServiceStatusObserverInterface.h"
#include "ServiceMessageReceiver.h"

namespace maf {
namespace messaging {

class ServiceRequesterInterface:
    public ServiceMessageReceiver,
    public ServiceStatusObserverInterface
{
};

}
}
