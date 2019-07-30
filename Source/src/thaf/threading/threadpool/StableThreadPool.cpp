#include <thread>
#include <vector>
#include "thaf/threading/Queue.h"
#include "thaf/threading/internal/StableThreadPool.h"
#include "thaf/threading/internal/ThreadPoolImplBase.h"

namespace thaf {
namespace threading {

struct __I
{
    using TaskExcFunc = typename ThreadPoolImplBase<threading::Queue<Runnable*> >::TaskExc;
    __I(unsigned int threadCount,  TaskExcFunc runFunc, TaskExcFunc stopFunc, TaskExcFunc doneFunc) :
        thepool(threadCount, runFunc, stopFunc, doneFunc)
    {
    }
    ThreadPoolImplBase<threading::Queue<Runnable*> >*  operator->()
    {
        return &thepool;
    }

    ThreadPoolImplBase<threading::Queue<Runnable*> > thepool;
};

StableThreadPool::StableThreadPool(unsigned int threadCount) :
    _pI(new __I{threadCount, &threading::run, &threading::stop, &threading::done})
{

    for(unsigned int i = 0; i < (*_pI)->maxThreadCount(); ++i)
    {
        (*_pI)->tryLaunchNewThread();
    }
}

StableThreadPool::~StableThreadPool()
{
    shutdown();
    delete _pI;
}

void StableThreadPool::run(Runnable *pRuner, unsigned int /*priority*/)
{
    (*_pI)->run(pRuner);
}

void StableThreadPool::setMaxThreadCount(unsigned int /*nThreadCount*/)
{

}

unsigned int StableThreadPool::activeThreadCount()
{
    return (*_pI)->activeThreadCount();
}

void StableThreadPool::shutdown()
{
    (*_pI)->shutdown();
}

}
}
