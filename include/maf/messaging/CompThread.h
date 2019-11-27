#pragma once

#include "Component.h"
#include <maf/threading/Thread.h>
#include <maf/export/MafExport_global.h>

namespace maf {
namespace messaging {

class CompThread : protected maf::threading::Thread
{
public:
    using Thread::Thread;
    using Thread::operator=;
    using Thread::join;
    using Thread::detach;
    using Thread::joinable;
    MAF_EXPORT CompThread& start();
};
} //messaging
} //maf
