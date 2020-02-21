#pragma once

namespace maf {
namespace messaging {

enum class Availability : char
{
    Unavailable,
    Available,
    Unknown
};

enum class ActionCallStatus : char
{
    Success,
    InvalidCall,
    InvalidParam,
    ServiceUnavailable,
    ReceiverBusy,
    ReceiverUnavailable,
    Timeout,
    FailedUnknown
};



}
}
