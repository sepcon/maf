#ifndef MESSAGEVALIDATOR_H
#define MESSAGEVALIDATOR_H

#include "thaf/messaging/client-server/Address.h"

namespace thaf {
namespace messaging {
namespace ipc {

class MessageValidator
{
public:
    virtual bool isValidAddress(const Address& /*addr*/) { return true; }
};
}// ipc
}// messaging
}// thaf
#endif // MESSAGEVALIDATOR_H
