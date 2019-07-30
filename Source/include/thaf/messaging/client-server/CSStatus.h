#pragma once

namespace thaf {
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
