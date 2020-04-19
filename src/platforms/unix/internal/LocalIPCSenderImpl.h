#pragma once

#include "SocketShared.h"
#include <maf/messaging/client-server/CSStatus.h>
#include <maf/utils/serialization/BASerializer.h>

namespace maf {
namespace messaging {
namespace ipc {

class LocalIPCSenderImpl {
public:
  LocalIPCSenderImpl();
  ~LocalIPCSenderImpl();
  ActionCallStatus send(const maf::srz::ByteArray &payload,
                        const Address &destination);
  bool initConnection(const Address &receiverAddr);
  Availability checkReceiverStatus() const;
  const Address &receiverAddress() const;

private:
  Address _myReceiverAddr;
  std::unique_ptr<SocketPath> _myReceiverSocketPath;
  std::unique_ptr<sockaddr_un> _myReceiverSockAddr;
};

} // namespace ipc
} // namespace messaging
} // namespace maf
