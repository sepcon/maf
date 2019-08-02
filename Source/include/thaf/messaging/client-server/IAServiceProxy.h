#pragma once

#include "SCQServiceProxy.h"
#include "IAMessageRouter.h"
#include "IAMessageTrait.h"

namespace thaf {
namespace messaging {

using IAServiceProxy = SCQServiceProxy<IAMessageTrait, IAMessageRouter>;

} // messaging
} // thaf
