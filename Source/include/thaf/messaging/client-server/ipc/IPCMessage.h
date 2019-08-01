#ifndef IPCMESSAGE_H
#define IPCMESSAGE_H

#include "thaf/utils/serialization/ByteArray.h"
#include "thaf/utils/serialization/SerializableInterface.h"
#include "thaf/utils/debugging/Debug.h"
#include "thaf/messaging/client-server/CSMessage.h"
#include <functional>

namespace thaf {
namespace messaging {
namespace ipc {

using PayloadType = srz::ByteArray;
class SerializableMessageContentBase : public CSMessageContentBase, public srz::SerializableInterface
{
public:
    srz::ByteArray toBytes() override { return {}; }
    void fromBytes(const srz::ByteArray& /*ba*/) override {}
    void makesureTransferable() override {
        _payload = toBytes();
    }
    PayloadType& payload() { return _payload; }
    const PayloadType& payload() const { return _payload; }
    void setPayload(PayloadType pl) { _payload = std::move(pl); }

protected:
    PayloadType _payload;
};

class IPCMessage : public CSMessage
{
public:
    using CSMessage::CSMessage;
    srz::ByteArray toBytes();
    bool fromBytes(const std::shared_ptr<srz::ByteArray>& bytes) noexcept;
};


}
}
}
#endif // IPCMESSAGE_H

