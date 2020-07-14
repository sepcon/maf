#pragma once

#include "CSMessage.h"
#include "CSMessageReceiverIF.h"
#include <functional>

namespace maf {
namespace messaging {

using CSMessageHandlerCallback = std::function<void(const CSMessagePtr &)>;

class MAF_EXPORT ServiceMessageReceiver : public CSMessageReceiverIF {
public:


protected:
  ServiceID _serviceID;
};

} // namespace messaging
} // namespace maf
