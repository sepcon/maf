#pragma once

#include <cassert>
#include <maf/messaging/client-server/CSMessage.h>
#include <maf/utils/serialization/SerializableIF.h>

namespace maf {
namespace messaging {

// clang-format off
struct cs_translatable  {};

struct cs_operation : public cs_translatable
{
    std::string dump() { return "cs_operation"; }
};

struct cs_param : public cs_translatable
{
    std::string dump() { return "cs_param"; }
};


struct cs_request           : public cs_operation   {};
struct cs_property          : public cs_operation   {};
struct cs_signal            : public cs_operation   {};

struct cs_inputbase         : public cs_param       {};
struct cs_input             : public cs_inputbase   {};

struct cs_outputbase        : public cs_param       {};
struct cs_output            : public cs_outputbase  {};
struct cs_status            : public cs_outputbase  {};
struct cs_attributes        : public cs_outputbase  {};

// clang-format on

template <class SerializableCSParamClass, class cs_param_type>
struct serializable_cs_param_t : public cs_param_type{ };

} // namespace messaging
} // namespace maf
