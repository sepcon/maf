#pragma once

#include "BasicMessages.h"
#include <maf/utils/serialization/SBObjectDefHlp.mc.h>

#define mc_dcl_message_start(MessageName) mc_sbClass_hasbase(MessageName, maf::messaging::MessageBase)
#define mc_dcl_message_end mc_sbClass_end
#define mc_dcl_msg_props mc_sbProperties
