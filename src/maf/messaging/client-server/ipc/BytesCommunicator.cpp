#include "BytesCommunicator.h"
#include "CommunicatorFactory.h"
#include <cassert>
#include <maf/logging/Logger.h>

namespace maf {

namespace messaging {
namespace ipc {

BytesCommunicator::~BytesCommunicator() { deinit(); }

BytesCommunicator::BytesCommunicator(IPCType type,
                                     CSMessageReceiverIF *receiver,
                                     bool isClient)
    : pSender_{CommunicatorFactory::createSender(type)},
      pReceiver_{CommunicatorFactory::createReceiver(type)},
      ipcMsgReceiver_(receiver), isClient_(isClient) {}

bool BytesCommunicator::init(const Address &serverAddress) {
  bool success = true;
  pReceiver_->registerObserver(this);
  success &= pSender_->initConnection(serverAddress);
  success &= pReceiver_->initConnection(serverAddress, isClient_);
  success &= pReceiver_->startListening();
  return success;
}

bool BytesCommunicator::deinit() {
  if (isWaiting()) {
    pReceiver_->stopListening();
  }
  return true;
}

bool BytesCommunicator::isWaiting() const {
  return (pReceiver_ && pReceiver_->listening());
}

ActionCallStatus BytesCommunicator::send(const std::shared_ptr<IPCMessage> &msg,
                                         const Address &recvAddr) {
  assert(msg != nullptr);
  if (pSender_) {
    try {
      if (recvAddr.valid()) {
        return pSender_->send(msg->toBytes(), recvAddr);
      } else {
        return pSender_->send(msg->toBytes(), {});
      }
    } catch (const std::bad_alloc &e) {
      MAF_LOGGER_ERROR("Message is too large to be serialized: ", e.what());
      return ActionCallStatus::FailedUnknown;
    }
  } else {
    MAF_LOGGER_ERROR("Cannot send message due to null sender, please call init "
                     "function before send function");
    return ActionCallStatus::ReceiverUnavailable;
  }
}

void BytesCommunicator::onBytesCome(
    const std::shared_ptr<srz::ByteArray> &bytes) {
  std::shared_ptr<IPCMessage> csMsg = std::make_shared<IPCMessage>();
  if (csMsg->fromBytes(bytes)) {
    if (isClient_ && csMsg->sourceAddress().valid()) {
      assert(csMsg->sourceAddress() == pReceiver_->address());
    }
    ipcMsgReceiver_->onIncomingMessage(csMsg);
  } else {
    MAF_LOGGER_ERROR("incoming message is not wellformed", "\n:The bytes are:[",
                     bytes->size(), "] ", *bytes);
  }
}

} // namespace ipc
} // namespace messaging
} // namespace maf
