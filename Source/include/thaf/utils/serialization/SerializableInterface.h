#pragma once

#include "ByteArray.h"

namespace thaf {
namespace srz {

class SerializableInterface
{
public:
    virtual ~SerializableInterface() = default;
    virtual srz::ByteArray toBytes() noexcept = 0;
    virtual void fromBytes(const srz::ByteArray& ba) = 0;
};

} // srz
} // thaf
