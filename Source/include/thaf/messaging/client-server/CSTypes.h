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
    Register,                           // register to get update every time properties' status has changed
    UnRegister,                         //
    Request,                            // Send request to server (Doing actions/get properties' status...)
    RequestSync,                        // Send request to server and the requester will be blocked until receiving the result
    Abort,                              // Ask server to abort pending/ongoing request that has not been done yet
    StatusUpdate,                       //
    RequestResultUpdate,                //
    RequestSyncResultUpdate,            //
    RequestResultDone,                  // apply for server to complete a request MANY updates from server until the request is fully respond. the callers must
    RequestSyncResultDone,              //
    RequestError,                       //
    SyncRequestError,                   //
    ServiceStatusUpdate,                // registering from client to ask for update status of each service whenever its status has changed
//  Unhandle                            // might be use to update client that the action is not applicable
    Invalid                             //
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
        "StatusUpdate",
        "RequestResultUpdate",
        "RequestSyncResultUpdate",
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
