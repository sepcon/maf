#include "maf/messaging/client-server/ipc/BytesCommunicator.h"
#include "maf/messaging/client-server/ipc/IPCFactory.h"
#include "maf/utils/debugging/Debug.h"
#include <cassert>


namespace maf {
namespace messaging {
namespace ipc {

BytesCommunicator::~BytesCommunicator()
{
    deinit();
}

BytesCommunicator::BytesCommunicator(CSMessageReceiver *receiver) :
    _ipcMsgReceiver(receiver)
{

}

void BytesCommunicator::init(IPCType type, const Address &serverAddress, bool isClient)
{
    _isClient = isClient;
    _pSender = IPCFactory::createSender(type);
    _pReceiver = IPCFactory::createReceiver(type);

    assert(_pSender && _pReceiver);

    _pSender->initConnection(serverAddress);
    _pReceiver->initConnection(serverAddress, isClient);
    _pReceiver->registerObserver(this);
    startWaitingMessages();
}

void BytesCommunicator::deinit()
{
    if(_pReceiver && _pReceiver->listening())
    {
        _pReceiver->stopListening();
    }
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
		try
		{
            if(recvAddr.valid())
            {
                return _pSender->send(msg->toBytes(), recvAddr);
            }
            else
            {
                return _pSender->send(msg->toBytes());
            }
		}
		catch (const std::bad_alloc& e)
		{
			mafErr("Message is too large to be serialized: " << e.what());
            return DataTransmissionErrorCode::FailedUnknown;
		}
    }
    else
    {
        mafErr("Cannot send message due to null sender, please call init function before send function");
        return DataTransmissionErrorCode::ReceiverUnavailable;
    }
}

void BytesCommunicator::onBytesCome(const std::shared_ptr<srz::ByteArray>& bytes)
{
    std::shared_ptr<IPCMessage> csMsg = std::make_shared<IPCMessage>();
    if(csMsg->fromBytes(bytes))
    {
        if(_isClient && csMsg->sourceAddress() != Address::INVALID_ADDRESS)
        {
            assert(csMsg->sourceAddress() == _pReceiver->address());
        }
        _ipcMsgReceiver->onIncomingMessage(csMsg);
    }
    else
    {
        mafErr("incoming message is not wellformed" << "\n:The bytes are:[" << bytes->size() << "] " << *bytes);
    }
}

} // ipc
} // messaging
} // maf
