#pragma once

#include <thaf/utils/IDManager.h>
#include <stdint.h>
#include <ostream>

namespace thaf {
namespace messaging {

using OpID = uint32_t;
using ServiceID = uint32_t;
using RequestID = util::IDManager::IDType;


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
//    UpdateStatus,
//    RequestResult,
//    SyncRequestResult,
//    RequestError,
//    SyncRequestError,
    ServiceStatusUpdate,
//  Unhandle
    Invalid
};

constexpr OpID OpIDInvalid = static_cast<OpID>(-1);
constexpr ServiceID ServiceIDInvalid = static_cast<ServiceID>(-1);
constexpr RequestID RequestIDInvalid = util::IDManager::INVALID_ID;
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


inline std::ostream& operator<<(std::ostream& ostr, OpCode code)
{
    return ostr << OpCodeString[static_cast<int>(code)];
}



} // messaging
} // thaf
