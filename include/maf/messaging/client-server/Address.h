#pragma once

#include <maf/utils/serialization/MafObjectBegin.mc.h>

namespace maf {
namespace messaging {

OBJECT(Address)
    public:
        using Port = uint16_t;
        using Name = std::string;
        using NameConstant = const char*;

        static constexpr Port INVALID_PORT = static_cast<Port>(-1);
        static constexpr NameConstant INVALID_NAME = "";
        bool valid() const
        {
            return (get_port() != INVALID_PORT) || (get_name() != INVALID_NAME);
        }

    MEMBERS
        (
            (Name, name, INVALID_NAME),
            (Port, port, INVALID_PORT)
        )

ENDOBJECT(Address)

}// messaging
}// maf

#include <maf/utils/serialization/MafObjectEnd.mc.h>
