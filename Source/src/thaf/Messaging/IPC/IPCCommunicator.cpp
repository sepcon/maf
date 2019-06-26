#include "thaf/Messaging/IPC/IPCCommunicator.h"
#include "thaf/Messaging/IPC/IPCFactory.h"
#include "thaf/Utils/Debugging/Debug.h"
#include <cassert>


namespace thaf {
namespace messaging {
namespace ipc {

IPCCommunicator::~IPCCommunicator()
{
   deinit();
}

void IPCCommunicator::init(IPCType ipcType, Address receiverAddr, bool isClient)
{
    _address = std::move(receiverAddr);
    _pSender = IPCFactory::createSender(ipcType);
    _pReceiver = IPCFactory::createReceiver(ipcType);

    assert(_pSender && _pReceiver);

    _pSender->initConnection(_address);
    _pReceiver->initConnection(_address, isClient);
    _pReceiver->registerObserver(this);
    startWaitingMessages();
}

void IPCCommunicator::startWaitingMessages()
{
    if(_pReceiver && !_pReceiver->listening())
    {
        _pReceiver->startListening();
    }
}

void IPCCommunicator::stopWaitingMessages()
{
    if(_pReceiver && _pReceiver->listening())
    {
        _pReceiver->stopListening();
    }
}

bool IPCCommunicator::isWaiting()
{
    return (_pReceiver && _pReceiver->listening());
}

ConnectionErrorCode IPCCommunicator::send(const std::shared_ptr<IPCMessage> &msg, const Address &destAddr)
{
    assert(msg != nullptr);
    if(_pSender)
    {
        if(destAddr != Address::INVALID_ADDRESS) { _pSender->initConnection(destAddr); }
        srz::BASerializer sr;
        sr << msg;
        return _pSender->send(sr.bytes());
    }
    else
    {
        thafErr("Cannot send message due to null sender, please call init function before send function");
        return ConnectionErrorCode::Failed;
    }
}

void IPCCommunicator::deinit()
{
    if(_pReceiver && _pReceiver->listening())
    {
        _pReceiver->stopListening();
    }
}

void IPCCommunicator::onBytesCome(const std::shared_ptr<srz::ByteArray>& bytes)
{
    std::shared_ptr<IPCMessage> ipcMsg; // = std::make_shared<IPCMessage>();
    try
    {
        srz::BADeserializer dsr(*bytes);
        dsr >> ipcMsg;
        if(ipcMsg && ipcMsg->wellformed())
        {
            onIPCMessage(ipcMsg);
        }
        else
        {
            thafErr("incoming message is not wellformed" << "\n:The bytes are:[" << bytes->size() << "] " << *bytes);
        }
    }
    catch (const std::exception& e)
    {
        thafErr("Error: Corruption of incoming bytes: " << "\nException: " << e.what());
    }
}

} // ipc
} // messaging
} // thaf
