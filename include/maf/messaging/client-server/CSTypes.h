#pragma once

#include <maf/utils/IDManager.h>
#include <stdint.h>
#include <string>

namespace maf {
namespace messaging {


using OpID              = std::string;
using OpIDConstant      = const char*;
using ServiceID         = uint32_t;
using RequestID         = util::IDManager::IDType;
using ConnectionType    = std::string;


enum class OpCode : unsigned char
{
//  Client Request
    Register,                           // \__
    UnRegister,                         // \__
    StatusGet,                          // \__ OpCodes for property status GET/BROADCAST
    Request,                            // Send INPUT to server (Doing actions/get properties' status...)
    Abort,                              // Ask server to ABORT pending/ongoing INPUT that has not been done yet
//  Server Response
    RequestError,                       // \__
    SyncRequestError,                   // \__
    RegisterServiceStatus,              // used by client to REGISTER for service status change event
    UnregisterServiceStatus,            // used by client to UNREGISTER for service status change event
    ServiceStatusUpdate,                // used by server to update service status to client
//  Unhandle                            // might be use to update client that the action is not applicable
    Invalid                             //
};

static constexpr OpIDConstant   OpIDInvalid         = "";
static constexpr ServiceID      ServiceIDInvalid    = static_cast<ServiceID>(-1);
static constexpr RequestID      RequestIDInvalid    = util::IDManager::INVALID_ID;


} // messaging
} // maf

