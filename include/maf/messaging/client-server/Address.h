#pragma once

#define MAF_ENABLE_DUMP
#include <maf/utils/serialization/Serializer.h>
#include <maf/utils/serialization/TupleLikeObject.mc.h>

namespace maf {
namespace messaging {

mc_tpl_class(Address)
    public:
        using Port = int32_t;
        using Name = std::string;
        static constexpr Port INVALID_PORT = -1;
        static const Name INVALID_NAME;
        static const Address INVALID_ADDRESS;
        bool valid() const { return (port() != INVALID_PORT) || (name() != INVALID_NAME);}
    mc_tpl_properties
        (
            (Name, name, INVALID_NAME),
            (Port, port, INVALID_PORT)
        )

mc_tpl_class_end(Address)


}// messaging
}// maf
