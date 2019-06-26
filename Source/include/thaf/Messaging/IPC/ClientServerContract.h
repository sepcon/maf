#ifndef CLIENTSERVERCONTRACT_H
#define CLIENTSERVERCONTRACT_H

#include "thaf/Utils/Serialization/SerializableObject.h"
#include "thaf/Patterns/Patterns.h"

namespace thaf {
namespace messaging {
namespace ipc {


using OpID = uint32_t;
static constexpr OpID OpIDInvalid = static_cast<OpID>(-1);

enum class OpCode : unsigned char
{
//  Client Request
    Register,
    UnRegister,
    Get,
    Set,
    Request,
    RequestSync,
    Abort,
//  Server Response
    UpdateStatus,
    RequestResult,
    SyncRequestResult,
    RequestError,
    SyncRequestError,
//  Unhandle
    Invalid
};

constexpr const char* OpCodeString[] =
{
    //  Client Request
        "Register",
        "UnRegister",
        "Get",
        "Set",
        "Request",
        "RequestSync",
        "Abort",
    //  Server Response
        "UpdateStatus",
        "RequestResult",
        "SyncRequestResult",
        "RequestError",
        "SyncRequestError",
    //  Unhandle
        "Invalid"
};

template<class OStream>
OStream& operator<<(OStream& ostr, OpCode code)
{
    return ostr << OpCodeString[static_cast<int>(code)];
}

class IPCDataCarrier : public pattern::UnCopyable
{
public:
    static OpID ID() { return OpIDInvalid; }
    virtual OpID getID() const = 0;
    virtual ~IPCDataCarrier() = default;
    virtual srz::ByteArray toBytes() const = 0;
    virtual void fromBytes(const srz::ByteArray& bytes) = 0;
};

}
}
}

#endif // CLIENTSERVERCONTRACT_H
