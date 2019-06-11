#ifndef IPCCOMMUNICATOR_H
#define IPCCOMMUNICATOR_H

#include "headers/Framework/Application/AppComponent.h"
#include "headers/Framework/Messaging/Message.h"
#include "Address.h"
#include <memory>

namespace thaf {
namespace srz { struct ByteArray; }
namespace messaging {

class MessageHandler;

namespace ipc {

class MessageValidator;
class IPCSender;
class IPCReceiver;

using OperationID = int;
enum struct OperationCode : char
{
    REGISTER,
    UNREGISTER,
    ACTION_REQUEST,
    ACTION_RESULT,
    STATUS_UPDATE,
    STATUS_SET,
    INVALID
};


class IPCMessage : public messaging::MessageBase
{
public:

private:
    std::shared_ptr<srz::ByteArray> _payload;
    Address _sourceAddress;
    OperationID _opID;
    OperationCode _opCode;

};

class IPCCommunicator : public app::AppComponent
{
protected:
    std::unique_ptr<IPCSender> _pSender;
    std::unique_ptr<IPCReceiver> _pReceiver;
    std::unique_ptr<MessageValidator> _pValidator;
};
}// ipc
}// messaging
}// thaf
#endif // IPCCOMMUNICATOR_H
