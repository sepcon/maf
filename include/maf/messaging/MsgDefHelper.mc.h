#pragma once

#include "BasicMessages.h"
#include <maf/utils/serialization/TupleLikeObject.mc.h>

#define mc_dcl_msg_start(MessageName) mc_sbClass_hasbase(MessageName, maf::messaging::MessageBase)
#define mc_dcl_msg_end mc_sbClass_end
#define mc_dcl_msg_props mc_sbProperties

#define mc_dcl_signal(SignalName) struct SignalName : public maf::messaging::MessageBase {\
}
