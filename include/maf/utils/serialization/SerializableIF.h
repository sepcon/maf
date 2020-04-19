#pragma once

#include "OByteStream.h"
#include "IByteStream.h"

namespace maf {
namespace srz {

class SerializableIF {
public:
  virtual ~SerializableIF() = default;
  virtual bool serialize(OByteStream &) const = 0;
  virtual bool deserialize(IByteStream &) = 0;
};

} // namespace srz
} // namespace maf
