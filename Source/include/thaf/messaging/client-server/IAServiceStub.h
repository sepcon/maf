#pragma once

#include "QueueingServiceStub.h"
#include "IAMessageTrait.h"
#include "IAMessageRouter.h"

namespace thaf {
namespace messaging {

using IARequestMesasge = ClientRequestMessage<IAMessageTrait>;
using IAServiceStub = QueueingServiceStub<IAMessageTrait>;

}
}
