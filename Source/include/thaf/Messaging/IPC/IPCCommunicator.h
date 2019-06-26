#ifndef IPCCOMMUNICATOR_H
#define IPCCOMMUNICATOR_H

#include "thaf/Application/AppComponent.h"
#include "thaf/Messaging/Message.h"
#include "thaf/Messaging/IPC/IPCReceiver.h"
#include "thaf/Messaging/IPC/IPCSender.h"
#include "thaf/Messaging/IPC/MessageValidator.h"
#include "IPCFactory.h"
#include "Address.h"
#include "IPCMessage.h"
#include <memory>

namespace thaf {
namespace srz { struct ByteArray; }
namespace messaging {
namespace ipc {

class IPCCommunicator : BytesComeObserver
{
public:
    void init(IPCType ipcType, Address receiverAddr, bool isClient = false);
    virtual void startWaitingMessages();
    virtual void stopWaitingMessages();
    virtual bool isWaiting();
    virtual void onIPCMessage(const std::shared_ptr<IPCMessage>& msg) = 0;
    virtual void deinit();
    ConnectionErrorCode send(const std::shared_ptr<IPCMessage>& msg, const Address &destAddr = {});
protected:
    ~IPCCommunicator() override; //Not allow to make instance of this class
    void onBytesCome(const std::shared_ptr<srz::ByteArray>& bytes) override;
    std::shared_ptr<IPCSender> _pSender;
    std::shared_ptr<IPCReceiver> _pReceiver;
    Address _address;
};

}// ipc
}// messaging
}// thaf
#endif // IPCCOMMUNICATOR_H
