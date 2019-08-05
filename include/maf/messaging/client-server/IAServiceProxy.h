#pragma once

#include "SCQServiceProxy.h"
#include "IAMessageRouter.h"
#include "IAMessageTrait.h"

namespace maf {
namespace messaging {

using IAServiceProxy = SCQServiceProxy<IAMessageTrait, IAMessageRouter>;

} // messaging
} // maf
