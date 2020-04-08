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
    : _pSender{CommunicatorFactory::createSender(type)},
      _pReceiver{CommunicatorFactory::createReceiver(type)},
      _ipcMsgReceiver(receiver), _isClient(isClient) {}

bool BytesCommunicator::init(const Address &serverAddress) {
  bool success = true;
  _pReceiver->registerObserver(this);
  success &= _pSender->initConnection(serverAddress);
  success &= _pReceiver->initConnection(serverAddress, _isClient);
  success &= _pReceiver->startListening();
  return success;
}

bool BytesCommunicator::deinit() {
  if (isWaiting()) {
    _pReceiver->stopListening();
  }
  return true;
}

bool BytesCommunicator::isWaiting() const {
  return (_pReceiver && _pReceiver->listening());
}

ActionCallStatus BytesCommunicator::send(const std::shared_ptr<IPCMessage> &msg,
                                         const Address &recvAddr) {
  assert(msg != nullptr);
  if (_pSender) {
    try {
      if (recvAddr.valid()) {
        return _pSender->send(msg->toBytes(), recvAddr);
      } else {
        return _pSender->send(msg->toBytes(), {});
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
    if (_isClient && csMsg->sourceAddress().valid()) {
      assert(csMsg->sourceAddress() == _pReceiver->address());
    }
    _ipcMsgReceiver->onIncomingMessage(csMsg);
  } else {
    MAF_LOGGER_ERROR("incoming message is not wellformed", "\n:The bytes are:[",
                     bytes->size(), "] ", *bytes);
  }
}

} // namespace ipc
} // namespace messaging
} // namespace maf
