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

enum class DataTransmissionErrorCode : char
{
    Success,
    ReceiverBusy,
    ReceiverUnavailable,
    CantConnectToReceiver,
    FailedUnknown
};



}
}
