#ifndef IPCMESSAGE_H
#define IPCMESSAGE_H

#include "maf/utils/serialization/ByteArray.h"
#include "maf/utils/serialization/SerializableInterface.h"
#include "maf/utils/debugging/Debug.h"
#include "maf/messaging/client-server/CSMessage.h"
#include <functional>

namespace maf {
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

