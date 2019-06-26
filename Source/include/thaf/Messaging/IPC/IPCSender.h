#pragma once

#include "Connection.h"
#include <memory>

namespace thaf {
namespace srz { struct ByteArray; }
namespace messaging {
namespace ipc {

class IPCSender
{
public:
    virtual ~IPCSender() = default;
    virtual void initConnection(const struct Address&) = 0;
    virtual ConnectionErrorCode send(const srz::ByteArray& ba) = 0;
    virtual ConnectionStatus checkServerStatus() const = 0;
    virtual const Address& receiverAddress() const = 0;
//    virtual void stop() = 0;
};

}// ipc
}// messaging
}// thaf
