#pragma once

#include "SSQServiceStub.h"
#include "IAMessageTrait.h"
#include "IAMessageRouter.h"

namespace maf {
namespace messaging {

using IARequestMesasge = ClientRequestMessage<IAMessageTrait>;
using IAServiceStub = SSQServiceStub<IAMessageTrait, IAMessageRouter>;

}
}
