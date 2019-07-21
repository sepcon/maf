#pragma once

#include "QueueingServiceStub.h"
#include "prv/IAMessageTrait.h"
#include "IAMessageRouter.h"

namespace thaf {
namespace messaging {

using IARequestMesasge = ClientRequestMessage<IAMessageTrait, IAMessageRouter>;
using IAServiceStub = QueueingServiceStub<IAMessageTrait, IAMessageRouter>;

}
}
