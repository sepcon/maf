#ifndef IPCCOMMUNICATOR_H
#define IPCCOMMUNICATOR_H

#include "CommunicatorFactory.h"
#include "IPCMessage.h"
#include "IPCReceiver.h"
#include "IPCSender.h"
#include <maf/messaging/client-server/Address.h>
#include <maf/messaging/client-server/CSMessageReceiverIF.h>
#include <memory>

namespace maf {
namespace srz {
struct ByteArray;
}
namespace messaging {
namespace ipc {

class BytesCommunicator : BytesComeObserver {
public:
  BytesCommunicator(IPCType type, CSMessageReceiverIF *receiver, bool isClient);
  bool init(const Address &serverAddress);
  bool deinit();
  bool isWaiting() const;
  ActionCallStatus send(const std::shared_ptr<IPCMessage> &msg,
                        const Address &recvAddr = {});
  ~BytesCommunicator() override; // Not allow to make instance of this class

protected:
  void onBytesCome(const std::shared_ptr<srz::ByteArray> &bytes) override;
  std::unique_ptr<IPCSender> pSender_;
  std::unique_ptr<IPCReceiver> pReceiver_;
  CSMessageReceiverIF *ipcMsgReceiver_;
  bool isClient_ = false;
};

} // namespace ipc
} // namespace messaging
} // namespace maf
#endif // IPCCOMMUNICATOR_H
