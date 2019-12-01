#pragma once

#include <maf/utils/IDManager.h>
#include <stdint.h>
#include <string>

namespace maf {
namespace messaging {


using OpID              = std::string;
using OpIDConst         = const char*;
using ServiceID         = std::string;
using ServiceIDConst    = const char*;
using RequestID         = util::IDManager::IDType;
using ConnectionType    = std::string;


enum class OpCode : char
{
//  Client Request
    StatusRegister,
    SignalRegister,
    Unregister,
    StatusGet,
    Request,
    Abort,
    RegisterServiceStatus,
    UnregisterServiceStatus,
    ServiceStatusUpdate,
//  Unhandle
    Invalid
};

constexpr OpIDConst         OpIDInvalid      = "";
constexpr ServiceIDConst    ServiceIDInvalid = "";
constexpr RequestID         RequestIDInvalid = util::IDManager::INVALID_ID;


} // messaging
} // maf

