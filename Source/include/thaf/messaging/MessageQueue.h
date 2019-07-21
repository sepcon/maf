#pragma once

#include "MessageBase.h"
#include "thaf/threading/Queue.h"

namespace thaf {
namespace messaging {

#ifdef MESSAGING_BY_PRIORITY
    using MessageQueue = threading::PriorityQueue<messaging::MessageBasePtr, messaging::MessageBase::PriorityComp>;
#else
    using MessageQueue = threading::Queue<messaging::MessageBasePtr>;
#endif

} // messaging
} // thaf
