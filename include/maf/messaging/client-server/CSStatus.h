#pragma once

namespace maf {
namespace messaging {

enum class Availability : char
{
    Available,
    Unavailable,
    Busy,
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
    FailedUnknown
};



}
}
