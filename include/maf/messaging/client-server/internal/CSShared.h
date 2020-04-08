#pragma once

#include <maf/messaging/client-server/CSMessage.h>
#include <maf/messaging/client-server/CSStatus.h>

namespace maf {
namespace messaging {

using CSMessageContentHandlerCallback =
    std::function<void(const CSMsgContentBasePtr &)>;

}
} // namespace maf
