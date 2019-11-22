#pragma once

#include <maf/utils/IDManager.h>
#include <stdint.h>
#include <ostream>

namespace maf {
namespace messaging {

using OpID      = uint32_t;
using ServiceID = uint32_t;
using RequestID = util::IDManager::IDType;
using ConnectionType = std::string;



enum class OpCode : unsigned char
{
//  Client Request
    Register,                           // \__
    UnRegister,                         // \__
    StatusGet,                          // \__ OpCodes for property status GET/BROADCAST
    Request,                            // Send REQUEST to server (Doing actions/get properties' status...)
    Abort,                              // Ask server to ABORT pending/ongoing REQUEST that has not been done yet
//  Server Response
    RequestError,                       // \__
    SyncRequestError,                   // \__
    RegisterServiceStatus,              // used by client to REGISTER for service status change event
    UnregisterServiceStatus,            // used by client to UNREGISTER for service status change event
    ServiceStatusUpdate,                // used by server to update service status to client
//  Unhandle                            // might be use to update client that the action is not applicable
    Invalid                             //
};

constexpr OpID      OpIDInvalid         = static_cast<OpID>(-1);
constexpr ServiceID ServiceIDInvalid    = static_cast<ServiceID>(-1);
constexpr RequestID RequestIDInvalid    = util::IDManager::INVALID_ID;

template<typename EnumType, std::enable_if_t<std::is_enum_v<EnumType>, bool> = true>
inline std::ostream& operator<<(std::ostream& ostr, EnumType code)
{
    return ostr << static_cast<int>(code);
}



} // messaging
} // maf
