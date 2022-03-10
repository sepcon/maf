#pragma once

#include <memory>

namespace maf {
namespace messaging {
class ClientIF;
namespace ipc {
namespace local {

std::shared_ptr<ClientIF> makeClient();

}  // namespace local
}  // namespace ipc
}  // namespace messaging
}  // namespace maf
