#pragma once

#include <maf/utils/StringifyableEnum.h>

namespace maf {
namespace messaging {

// clang-format off
MC_MAF_STRINGIFYABLE_ENUM(Availability, char,
                    Unavailable,
                    Available,
                    Unknown
                          )

MC_MAF_STRINGIFYABLE_ENUM(ActionCallStatus, char,
                    Success,
                    InvalidCall,
                    InvalidParam,
                    ServiceUnavailable,
                    ReceiverBusy,
                    ReceiverUnavailable,
                    Timeout,
                    FailedUnknown)
// clang-format on
}
}
