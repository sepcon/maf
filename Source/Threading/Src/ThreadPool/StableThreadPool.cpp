#include <thread>
#include <vector>
#include "Interfaces/Queue.h"
#include "Prv/TP/StableThreadPool.h"


namespace Threading
{

namespace Internal {


void run(Runnable* runner)
{
    if(runner)
    {
        runner->run();
    }
}

void stop(Runnable* runner)
{
    if(runner)
    {
        runner->stop();
    }
}

void done(Runnable* runner)
{
    if(runner && runner->autoDelete())
    {
        delete runner;
    }
}
}

StableThreadPool::StableThreadPool(unsigned int threadCount) :
    _impl{threadCount, &Internal::run, &Internal::stop, &Internal::done}
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
