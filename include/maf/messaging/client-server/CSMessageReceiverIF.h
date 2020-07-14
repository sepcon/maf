#pragma once

#include "CSMessage.h"
#include <maf/export/MafExport_global.h>

namespace maf {
namespace messaging {

class MAF_EXPORT CSMessageReceiverIF {
public:
  virtual bool onIncomingMessage(const CSMessagePtr &csMsg) = 0;
  virtual ~CSMessageReceiverIF() = default;
};

} // namespace messaging
} // namespace maf
