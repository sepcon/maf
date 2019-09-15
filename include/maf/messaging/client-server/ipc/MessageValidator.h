#ifndef MESSAGEVALIDATOR_H
#define MESSAGEVALIDATOR_H

#include <maf/messaging/client-server/Address.h>

namespace maf {
namespace messaging {
namespace ipc {

class MessageValidator
{
public:
    virtual bool isValidAddress(const Address& /*addr*/) { return true; }
};
}// ipc
}// messaging
}// maf
#endif // MESSAGEVALIDATOR_H
