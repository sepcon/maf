#pragma once
#include "maf/utils/serialization/SerializableObject.h"
#include "maf/patterns/Patterns.h"

namespace maf {
namespace messaging {

//using OpID = uint32_t;
//using ServiceID = uint32_t;

//static constexpr OpID OpIDInvalid = static_cast<OpID>(-1);
//static constexpr ServiceID ServiceIDInvalid = static_cast<ServiceID>(-1);

//enum class OpCode : unsigned char
//{
////  Client Request
//    Register,
//    UnRegister,
//    Get,
//    Set,
//    Request,
//    RequestSync,
//    Abort,
////  Server Response
//    StatusUpdate,
//    RequestResultUpdate,
//    RequestSyncResultUpdate,
//    RequestError,
//    SyncRequestError,
////  Unhandle
//    Invalid
//};

//constexpr const char* OpCodeString[] =
//{
//    //  Client Request
//        "Register",
//        "UnRegister",
//        "Get",
//        "Set",
//        "Request",
//        "RequestSync",
//        "Abort",
//    //  Server Response
//        "StatusUpdate",
//        "RequestResultUpdate",
//        "RequestSyncResultUpdate",
//        "RequestError",
//        "SyncRequestError",
//    //  Unhandle
//        "Invalid"
//};

//template<class OStream>
//OStream& operator<<(OStream& ostr, OpCode code)
//{
//    return ostr << OpCodeString[static_cast<int>(code)];
//}

//class SerializableMessageContentBase : public pattern::UnCopyable
//{
//public:
//    static OpID ID() { return OpIDInvalid; }
//    virtual OpID getName() const = 0;
//    virtual ~SerializableMessageContentBase() = default;
//    virtual srz::ByteArray toBytes() const = 0;
//    virtual void fromBytes(const srz::ByteArray& bytes) = 0;
//};

}
}

