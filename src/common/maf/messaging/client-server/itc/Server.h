#pragma once

#include <maf/messaging/client-server/ServerIF.h>

namespace maf {
namespace messaging {
namespace itc {

std::shared_ptr<ServerIF> makeServer();

}  // namespace itc
}  // namespace messaging
}  // namespace maf
