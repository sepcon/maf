#ifndef IPCCOMMUNICATOR_H
#define IPCCOMMUNICATOR_H

#include "thaf/messaging/MessageBase.h"
#include "thaf/messaging/ipc/IPCReceiver.h"
#include "thaf/messaging/ipc/IPCSender.h"
#include "thaf/messaging/ipc/MessageValidator.h"
#include "thaf/messaging/client-server/interfaces/Address.h"
#include "thaf/messaging/client-server/interfaces/CSMessageReceiver.h"
#include "IPCFactory.h"
#include "IPCMessage.h"
#include <memory>

namespace thaf {
namespace srz { struct ByteArray; }
namespace messaging {
namespace ipc {

class BytesCommunicator : BytesComeObserver
{
public:
    BytesCommunicator(CSMessageReceiver* receiver);
    void init(IPCType type, const Address& serverAddress, bool isClient);
    void startWaitingMessages();
    void stopWaitingMessages();
    bool isWaiting() const;
    DataTransmissionErrorCode send(const std::shared_ptr<IPCMessage>& msg, const Address & recvAddr = {});
    ~BytesCommunicator() override; //Not allow to make instance of this class

protected:
    void onBytesCome(const std::shared_ptr<srz::ByteArray>& bytes) override;
    std::shared_ptr<IPCSender> _pSender;
    std::shared_ptr<IPCReceiver> _pReceiver;
    CSMessageReceiver* _ipcMsgReceiver;
    Address _address;
};

}// ipc
}// messaging
}// thaf
#endif // IPCCOMMUNICATOR_H
