#pragma once

namespace maf {
namespace messaging {

struct MessageTraitBase
{
    enum EncodeDecodeStatus
    {
        Success,
        EmptyInput,
        MalformInput
    };
    static void setStatus(EncodeDecodeStatus* output, EncodeDecodeStatus status)
    {
        if(output)
        {
            *output = status;
        }
    }
};
} // messaging
} // maf
