#include "Prv/TP/PriorityThreadPool.h"
namespace Threading
{

PriorityThreadPool::PriorityThreadPool(unsigned int threadCount) : _impl{threadCount}
{
}

PriorityThreadPool::~PriorityThreadPool()
{
    shutdown();
}

void PriorityThreadPool::run(Runnable *pRuner, unsigned int priority)
{
    _impl.run( { pRuner, priority } );
}


unsigned int PriorityThreadPool::activeThreadCount()
{
    return _impl.activeThreadCount();
}

void PriorityThreadPool::shutdown()
{
    _impl.shutdown();
}


}
