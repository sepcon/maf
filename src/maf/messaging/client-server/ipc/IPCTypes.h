#pragma once

namespace maf {
namespace messaging {
namespace ipc {

enum class IPCType : unsigned char {
  Local,
  Domain,
  // Somethings else will be defined here
  Invalid
};

} // namespace ipc
} // namespace messaging
} // namespace maf
