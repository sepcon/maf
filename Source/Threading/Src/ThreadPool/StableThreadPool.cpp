#include <thread>
#include <vector>
#include "Interfaces/Queue.h"
#include "Prv/TP/StableThreadPool.h"


namespace Threading
{

StableThreadPool::StableThreadPool(unsigned int threadCount) :
    _impl{ threadCount, &Threading::run, &Threading::stop, &Threading::done }
{
    for(unsigned int i = 0; i < _impl.maxThreadCount(); ++i)
    {
        _impl.tryLaunchNewThread();
    }
}

StableThreadPool::~StableThreadPool()
{
    _impl.shutdown();
}

void StableThreadPool::run(Runnable *pRuner, unsigned int /*priority*/)
{
    _impl.run(pRuner);
}

void StableThreadPool::setMaxThreadCount(unsigned int /*nThreadCount*/)
{

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
