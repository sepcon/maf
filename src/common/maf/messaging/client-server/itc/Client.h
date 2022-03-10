#pragma once

#include <maf/messaging/client-server/ClientIF.h>

namespace maf {
namespace messaging {
namespace itc {
std::shared_ptr<ClientIF> makeClient();
}  // namespace itc
}  // namespace messaging
}  // namespace maf
