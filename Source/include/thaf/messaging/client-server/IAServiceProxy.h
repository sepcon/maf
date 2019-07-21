#pragma once

#include "QueueingServiceProxy.h"
#include "IAMessageRouter.h"
#include "prv/IAMessageTrait.h"

namespace thaf {
namespace messaging {

using IAServiceProxy = QueueingServiceProxy<IAMessageTrait, IAMessageRouter>;

} // messaging
} // thaf
