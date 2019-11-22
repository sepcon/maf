#pragma once

#include <maf/messaging/client-server/CSStatus.h>
#include <maf/messaging/client-server/CSMessage.h>


namespace maf {
namespace messaging {

using RequestAbortedCallback          = std::function<void(void)>;
using CSMessageContentHandlerCallback = std::function<void(const CSMsgContentBasePtr&)>;


}
}
