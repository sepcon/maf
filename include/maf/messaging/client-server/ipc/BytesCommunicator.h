#ifndef IPCCOMMUNICATOR_H
#define IPCCOMMUNICATOR_H

#include <maf/messaging/MessageBase.h>
#include <maf/messaging/client-server/ipc/IPCReceiver.h>
#include <maf/messaging/client-server/ipc/IPCSender.h>
#include <maf/messaging/client-server/ipc/MessageValidator.h>
#include <maf/messaging/client-server/Address.h>
#include <maf/messaging/client-server/CSMessageReceiver.h>
#include "IPCFactory.h"
#include "IPCMessage.h"
#include <memory>

namespace maf {
namespace srz { struct ByteArray; }
namespace messaging {
namespace ipc {

class BytesCommunicator : BytesComeObserver
{
public:
    BytesCommunicator(IPCType type, CSMessageReceiver* receiver, bool isClient);
    bool init(const Address& serverAddress);
    bool deinit();
    bool isWaiting() const;
    DataTransmissionErrorCode send(const std::shared_ptr<IPCMessage>& msg, const Address & recvAddr = Address::INVALID_ADDRESS);
    ~BytesCommunicator() override; //Not allow to make instance of this class

protected:
    void onBytesCome(const std::shared_ptr<srz::ByteArray>& bytes) override;
    std::shared_ptr<IPCSender> _pSender;
    std::shared_ptr<IPCReceiver> _pReceiver;
    CSMessageReceiver* _ipcMsgReceiver;
    bool _isClient = false;
};

}// ipc
}// messaging
}// maf
#endif // IPCCOMMUNICATOR_H
