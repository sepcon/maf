#pragma once

#include "QueueingServiceProxy.h"
#include "IAMessageRouter.h"
#include "IAMessageTrait.h"

namespace thaf {
namespace messaging {

using IAServiceProxy = QueueingServiceProxy<IAMessageTrait>;

} // messaging
} // thaf
