#pragma once

#include <maf/threading/Thread.h>
#include "Component.h"

namespace maf {
namespace messaging {

class CompThread : protected maf::threading::Thread
{
public:
//    CompThread() = default;
//    CompThread(CompThread&& th);
//    CompThread& operator=(CompThread&& th);
    template<class Callable, class... Args>
    CompThread(Callable&& f, Args&&... args) : Thread{std::forward<Callable>(f), std::forward<Args>(args)...}{}
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
