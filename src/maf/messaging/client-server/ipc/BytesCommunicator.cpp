#include <maf/messaging/client-server/ipc/BytesCommunicator.h>
#include <maf/messaging/client-server/ipc/IPCFactory.h>
#include <maf/logging/Logger.h>
#include <cassert>


namespace maf { using logging::Logger;
namespace messaging {
namespace ipc {

BytesCommunicator::~BytesCommunicator()
{
    deinit();
}

BytesCommunicator::BytesCommunicator(IPCType type, CSMessageReceiver *receiver, bool isClient) :
    _ipcMsgReceiver(receiver),
    _isClient(isClient)
{
    _pSender = IPCFactory::createSender(type);
    _pReceiver = IPCFactory::createReceiver(type);
}

bool BytesCommunicator::init(const Address &serverAddress)
{
    bool success = true;
    _pReceiver->registerObserver(this);
    success &= _pSender->initConnection(serverAddress);
    success &= _pReceiver->initConnection(serverAddress, _isClient);
    success &= _pReceiver->startListening();
    return success;
}

bool BytesCommunicator::deinit()
{
    if(isWaiting())
    {
        _pReceiver->stopListening();
    }
    return true;
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
			Logger::error("Message is too large to be serialized: " ,  e.what());
            return DataTransmissionErrorCode::FailedUnknown;
		}
    }
    else
    {
        Logger::error("Cannot send message due to null sender, please call init function before send function");
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
        Logger::error("incoming message is not wellformed" ,  "\n:The bytes are:[" ,  bytes->size() ,  "] " ,  *bytes);
    }
}

} // ipc
} // messaging
} // maf
