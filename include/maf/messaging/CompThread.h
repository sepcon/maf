#pragma once

#include <maf/threading/Thread.h>
#include "Component.h"

namespace maf {
namespace messaging {

class CompThread : protected maf::threading::Thread
{
public:
    using Thread::operator=;
    using Thread::Thread;
    using Thread::join;
    using Thread::detach;
    using Thread::joinable;
    using Thread::setSignalHandler;
    CompThread& start();
};
} //messaging
} //maf
