#pragma once

#include "CompMessageBase.h"
#include <maf/threading/Queue.h>

namespace maf {
namespace messaging {

#ifdef MESSAGING_BY_PRIORITY
    using MessageQueue = threading::PriorityQueue<messaging::MessageBasePtr, messaging::CompMessageBase::PriorityComp>;
#else
    using MessageQueue = threading::Queue<messaging::MessageBasePtr>;
#endif

} // messaging
} // maf
