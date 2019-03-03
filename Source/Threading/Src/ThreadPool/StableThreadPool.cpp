#include <thread>
#include <vector>
#include "Interfaces/Queue.h"
#include "Prv/TP/StableThreadPool.h"





namespace Threading
{

StableThreadPool::StableThreadPool(unsigned int threadCount) : _impl{threadCount}
{
}

StableThreadPool::~StableThreadPool()
{
    _impl.shutdown();
}

void StableThreadPool::run(Runnable *pRuner, unsigned int /*priority*/)
{
    _impl.run(pRuner);
}

unsigned int StableThreadPool::activeThreadCount()
{
    return _impl.activeThreadCount();
}

void StableThreadPool::shutdown()
{
    _impl.shutdown();
}

}
