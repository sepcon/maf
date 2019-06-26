#ifndef IPCMESSAGE_H
#define IPCMESSAGE_H

#include "headers/Libs/Utils/Serialization/ByteArray.h"
#include "headers/Framework/Messaging/Message.h"

namespace thaf {
namespace messaging {
namespace ipc {


using OpID = int;
static inline constexpr OpID OpIDInvalid = 0;

enum class OpCode : unsigned char
{
    Reg,
    UnReg,
    Get,
    Set,
    Req,
    Rej,
    ResRslt,
    ResErr,
    Invalid
};


class IPCMessage : public MessageBase
{

public:
    IPCMessage();
    IPCMessage(OpCode code, OpID id, const srz::ByteArray& ba);
    IPCMessage(OpCode code, OpID id, srz::ByteArray&& ba);
    ~IPCMessage();
    void setPayload(const srz::ByteArray& ba);
    void setPayload(srz::ByteArray&& ba);
    const srz::ByteArray& getPayload() const;
    void setOpCode(const OpCode code);
    OpCode getOpCode() const;
    void setOpID(const OpID id);
    OpID getOpID() const;
    void fromBytes(const srz::ByteArray& ba);
    void fromBytes(const char* bytestring, size_t length);
    srz::ByteArray toBytes();
    bool wellformed() const;

private:
    srz::ByteArray _payload;
    OpCode _opCode;
    OpID _opID;
};

}
}
}
#endif // IPCMESSAGE_H
