#include "Prv/TP/PriorityThreadPool.h"
namespace Threading
{

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


namespace Internal {
static void run(PrioritizableRunner& runner)  ;
static void stop(PrioritizableRunner& runner) ;
static void done(PrioritizableRunner& runner) ;
}

struct TheImpl
{
    TheImpl(unsigned int threadCount = 0)
        : thePool { threadCount, &Internal::run, &Internal::stop, &Internal::done }
    {
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


namespace Internal
{

static void run(PrioritizableRunner& runner)
{
    auto pRunner = runner._pRunner;
    if(pRunner)
    {
        pRunner->run();
    }
}

static void stop(PrioritizableRunner& runner)
{
    auto pRunner = runner._pRunner;
    if(pRunner)
    {
        pRunner->stop();
    }
}

static void done(PrioritizableRunner& runner)
{
    auto pRunner = runner._pRunner;
    if(pRunner && pRunner->autoDelete())
    {
        delete pRunner;
    }
}

}
}
