#pragma once

#include <maf/utils/IDManager.h>
#include <stdint.h>
#include <ostream>

namespace maf {
namespace messaging {

using OpID = uint32_t;
using ServiceID = uint32_t;
using RequestID = util::IDManager::IDType;


enum class OpCode : unsigned char
{
//  Client Request
    Register,                           // register to get update every time properties' status has changed
    UnRegister,                         //
    Request,                            // Send request to server (Doing actions/get properties' status...)
    RequestSync,                        // Send request to server and the requester will be blocked until receiving the result
    Abort,                              // Ask server to abort pending/ongoing request that has not been done yet
//  Server Response
    StatusUpdate,                       // \__
    RequestResultUpdate,                // \__
    RequestSyncResultUpdate,            // \__
    RequestResultDone,                  // \__ Apply for server to complete a request MANY updates from server until the request is fully respond. the callers must
    RequestSyncResultDone,              // \__
    RequestError,                       // \__
    SyncRequestError,                   // \__
    RegisterServiceStatus,              // used by client to register for service status change event
    ServiceStatusUpdate,                // used by server to update service status to client
//  Unhandle                            // might be use to update client that the action is not applicable
    Invalid                             //
};

constexpr OpID OpIDInvalid = static_cast<OpID>(-1);
constexpr ServiceID ServiceIDInvalid = static_cast<ServiceID>(-1);
constexpr RequestID RequestIDInvalid = util::IDManager::INVALID_ID;

template<typename EnumType, std::enable_if_t<std::is_enum_v<EnumType>, bool> = true>
inline std::ostream& operator<<(std::ostream& ostr, EnumType code)
{
    return ostr << static_cast<int>(code);
}



} // messaging
} // maf
