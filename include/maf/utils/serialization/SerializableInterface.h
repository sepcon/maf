#pragma once

#include "ByteArray.h"

namespace maf {
namespace srz {

class SerializableInterface
{
public:
    virtual ~SerializableInterface() = default;
    virtual srz::ByteArray toBytes() = 0;
    virtual void fromBytes(const srz::ByteArray& ba) = 0;
};

} // srz
} // maf
