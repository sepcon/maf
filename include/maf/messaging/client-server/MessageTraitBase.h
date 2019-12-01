#pragma once

#include "internal/cs_param.h"

namespace maf {
namespace messaging {

struct MessageTraitBase
{
    enum CodecStatus
    {
        Success,
        EmptyInput,
        MalformInput
    };

    template <class T>
    static constexpr bool encodable()
    {
        return std::is_base_of_v<cs_inputbase, T> ||
               std::is_base_of_v<cs_outputbase, T>;
    }

    static void assignCodecStatus(CodecStatus* output, CodecStatus status)
    {
        if(output)
        {
            *output = status;
        }
    }
};
} // messaging
} // maf
