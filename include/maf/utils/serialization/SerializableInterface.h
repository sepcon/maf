#pragma once

#include "ByteArray.h"

namespace maf {
namespace srz {

class SerializableIF {
public:
  virtual ~SerializableIF() = default;
  virtual srz::ByteArray toBytes() = 0;
  virtual void fromBytes(const srz::ByteArray &ba) = 0;
};

} // namespace srz
} // namespace maf
