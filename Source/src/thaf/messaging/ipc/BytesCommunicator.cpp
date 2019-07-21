#include "thaf/messaging/ipc/BytesCommunicator.h"
#include "thaf/messaging/ipc/IPCFactory.h"
#include "thaf/utils/debugging/Debug.h"
#include <cassert>


namespace thaf {
namespace messaging {
namespace ipc {

BytesCommunicator::~BytesCommunicator()
{
    if(_pReceiver && _pReceiver->listening())
    {
        _pReceiver->stopListening();
    }
}

BytesCommunicator::BytesCommunicator(CSMessageReceiver *receiver) :
    _ipcMsgReceiver(receiver)
{

}

void BytesCommunicator::init(IPCType type, const Address &serverAddress, bool isClient)
{
    _address = std::move(serverAddress);
    _pSender = IPCFactory::createSender(type);
    _pReceiver = IPCFactory::createReceiver(type);

    assert(_pSender && _pReceiver);

    _pSender->initConnection(_address);
    _pReceiver->initConnection(_address, isClient);
    _pReceiver->registerObserver(this);
    startWaitingMessages();
}

void BytesCommunicator::startWaitingMessages()
{
    if(_pReceiver && !_pReceiver->listening())
    {
        _pReceiver->startListening();
    }
}

void BytesCommunicator::stopWaitingMessages()
{
    if(_pReceiver && _pReceiver->listening())
    {
        _pReceiver->stopListening();
    }
}

bool BytesCommunicator::isWaiting() const
{
    return (_pReceiver && _pReceiver->listening());
}


DataTransmissionErrorCode BytesCommunicator::send(const std::shared_ptr<IPCMessage> &msg, const Address &recvAddr)
{
    assert(msg != nullptr);
    if(_pSender)
    {
        if(recvAddr != Address::INVALID_ADDRESS) { _pSender->initConnection(recvAddr); }
        return _pSender->send(msg->toBytes());
    }
    else
    {
        thafErr("Cannot send message due to null sender, please call init function before send function");
        return DataTransmissionErrorCode::ReceiverUnavailable;
    }
}

void BytesCommunicator::onBytesCome(const std::shared_ptr<srz::ByteArray>& bytes)
{
    std::shared_ptr<IPCMessage> csMsg = std::make_shared<IPCMessage>();
    if(csMsg->fromBytes(bytes))
    {
        _ipcMsgReceiver->onIncomingMessage(csMsg);
    }
    else
    {
        thafErr("incoming message is not wellformed" << "\n:The bytes are:[" << bytes->size() << "] " << *bytes);
    }
}

} // ipc
} // messaging
} // thaf
