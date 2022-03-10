#pragma once

#include <maf/export/MafExport_global.h>

#include <ostream>

namespace maf {
namespace messaging {

enum class CSPayloadType : char {
  IncomingData = 0,
  OutgoingData = 1,
  Error = 2,
  NA
};

class MAF_EXPORT CSMsgPayloadIF {
 public:
  virtual ~CSMsgPayloadIF() = default;
  virtual CSPayloadType type() const = 0;
  virtual bool equal(const CSMsgPayloadIF *other) const = 0;
  virtual CSMsgPayloadIF *clone() const = 0;
  virtual void dump(std::ostream &) const = 0;
};

}  // namespace messaging
}  // namespace maf
