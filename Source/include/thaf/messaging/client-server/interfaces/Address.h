#pragma once

#include "thaf/utils/serialization/Serializer.h"
#include "thaf/utils/serialization/SerializableObject.h"

namespace thaf {
namespace messaging {

mc_sbClass(Address)
    public:
        using Port = int32_t;
        using Name = std::string;
        static constexpr Port INVALID_PORT = -1;
        constexpr static const char* const INVALID_NAME = "";
        static const Address INVALID_ADDRESS;

    mc_sbProperties
        (
            (Name, name, INVALID_NAME),
            (Port, port, static_cast<Port>(INVALID_PORT))
        )

mc_sbClass_end(Address)


}// messaging
}// thaf
