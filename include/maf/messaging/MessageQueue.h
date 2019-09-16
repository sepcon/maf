#pragma once

#include "MessageBase.h"
#include <maf/threading/Queue.h>

namespace maf {
namespace messaging {

#ifdef MESSAGING_BY_PRIORITY
    using MessageQueue = threading::PriorityQueue<messaging::MessageBasePtr, messaging::MessageBase::PriorityComp>;
#else
    using MessageQueue = threading::Queue<messaging::MessageBasePtr>;
#endif

} // messaging
} // maf
