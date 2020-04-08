#ifndef IPCMESSAGE_H
#define IPCMESSAGE_H

#include <maf/messaging/client-server/CSMessage.h>

namespace maf {
namespace messaging {
namespace ipc {

class IPCMessage : public CSMessage {
public:
  using CSMessage::CSMessage;
  srz::ByteArray toBytes();
  bool fromBytes(const std::shared_ptr<srz::ByteArray> &bytes) noexcept;
};

} // namespace ipc
} // namespace messaging
} // namespace maf
#endif // IPCMESSAGE_H
