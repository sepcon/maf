#pragma once

#include "CSMessage.h"

namespace maf {
namespace messaging {

class CSMessageReceiverIF {
public:
  virtual bool onIncomingMessage(const CSMessagePtr &csMsg) = 0;
};

} // namespace messaging
} // namespace maf
