#pragma once

#include <maf/export/MafExport_global.h>

namespace maf {
namespace messaging {
class CSMessage;

namespace ipc {

class MAF_EXPORT MessageValidatorIF {
public:
  virtual bool isValid(const CSMessage *) { return true; }
  virtua ~MessageValidatorIF() = default;
};

} // namespace ipc
} // namespace messaging
} // namespace maf
