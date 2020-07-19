#pragma once

#include <maf/messaging/client-server/CSMessage.h>
#include <maf/utils/serialization/Buffer.h>

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

class LocalIPCMessage : public CSMessage {
 public:
  using CSMessage::CSMessage;
  srz::Buffer toBytes();
  bool fromBytes(srz::Buffer &&bytes) noexcept;
};

}  // namespace local
}  // namespace ipc
}  // namespace messaging
}  // namespace maf
