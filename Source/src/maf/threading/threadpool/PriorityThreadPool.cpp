#include "maf/threading/internal/PriorityThreadPool.h"
#include "maf/threading/internal/ThreadPoolImplBase.h"
#include "maf/threading/Queue.h"

namespace maf {
namespace threading {

struct PrioritizableRunner
{
public:
    Runnable* _pRunner;
    unsigned int _priority;
    PrioritizableRunner(Runnable* pRunner = nullptr, unsigned int priority = 0) :
        _pRunner(pRunner),
        _priority(priority)
    {
    }
    friend bool operator<(const PrioritizableRunner& p1, const PrioritizableRunner& p2)
    {
        return p1._priority < p2._priority;
    }
    friend bool operator==(const PrioritizableRunner& p1, const PrioritizableRunner& p2)
    {
        return p1._pRunner == p2._pRunner && p1._priority == p2._priority;
    }
};

#define prAct(func) [](PrioritizableRunner& runner) { threading::func(runner._pRunner); }

struct TheImpl
{
    TheImpl(unsigned int threadCount = 0) : thePool { threadCount, prAct(run), prAct(stop), prAct(done)}
    {
        for(unsigned int i = 0; i < thePool.maxThreadCount(); ++i)
        {
            thePool.tryLaunchNewThread();
        }
    }

    ThreadPoolImplBase<PriorityQueue<PrioritizableRunner>> thePool;
};


PriorityThreadPool::PriorityThreadPool(unsigned int threadCount)
{
    _pImpl = new TheImpl(threadCount);
}

PriorityThreadPool::~PriorityThreadPool()
{
    delete _pImpl;
}

void PriorityThreadPool::run(Runnable *pRuner, unsigned int priority)
{
    _pImpl->thePool.run({pRuner, priority});
}


unsigned int PriorityThreadPool::activeThreadCount()
{
    return _pImpl->thePool.activeThreadCount();
}

void PriorityThreadPool::shutdown()
{
    _pImpl->thePool.shutdown();
}

}
}
