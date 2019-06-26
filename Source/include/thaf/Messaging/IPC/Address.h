#ifndef ADDRESS_H
#define ADDRESS_H

#include "thaf/Utils/Serialization/Serializer.h"
#include "thaf/Utils/Serialization/SerializableObject.h"
#include <string>

namespace thaf {
namespace messaging {
namespace ipc {

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


}// ipc
}// messaging
}// thaf
#endif // ADDRESS_H
