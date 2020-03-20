#pragma once

#include <maf/utils/IDManager.h>
#include <maf/utils/StringifyableEnum.h>
#include <stdint.h>
#include <string>

// clang-format off
namespace maf {
namespace messaging {


using OpID              = std::string;
using OpIDConst         = const char*;
using ServiceID         = std::string;
using ServiceIDConst    = const char*;
using RequestID         = util::IDManager::IDType;
using ConnectionType    = std::string;


MC_MAF_STRINGIFYABLE_ENUM(OpCode, char,
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
)

constexpr OpIDConst         OpIDInvalid      = "";
constexpr ServiceIDConst    ServiceIDInvalid = "";
constexpr RequestID         RequestIDInvalid = util::IDManager::INVALID_ID;
constexpr OpIDConst OpID_ServiceAvailable    = "service_available.property";
constexpr OpIDConst OpID_ServiceUnavailable  = "service_unavailable.property";

// clang-format on
} // messaging
} // maf
