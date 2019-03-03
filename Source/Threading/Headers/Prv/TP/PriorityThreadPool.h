#ifndef PRIORITYTHREADPOOL_H
#define PRIORITYTHREADPOOL_H

#include "Interfaces/IThreadPool.h"
#include "Interfaces/Queue.h"
#include "Prv/TP/ThreadPoolImplBase.h"

namespace Threading
{

class PriorityThreadPool : public IThreadPool
{
public:
    PriorityThreadPool(unsigned int threadCount = 0);
    ~PriorityThreadPool() override;
    virtual void run(Runnable* pRuner, unsigned int priority = 0) override;
    virtual void setMaxThreadCount(unsigned int /*nThreadCount*/) override {}
    virtual unsigned int activeThreadCount() override;
    virtual void shutdown() override;
private:
    class PrioritizableRunner
    {
        Runnable* _pRunner;
        unsigned int _priority;
    public:
        PrioritizableRunner(Runnable* pRunner = nullptr, unsigned int priority = 0) :
            _pRunner(pRunner),
            _priority(priority)
        {
        }
        void run()
        {
            if(_pRunner) _pRunner->run();
        }
        void stop()
        {
            if(_pRunner) _pRunner->stop();
        }
        friend bool operator<(const PrioritizableRunner& r1, const PrioritizableRunner& r2)
        {
            return r1._priority < r2._priority;
        }
        friend bool operator==(const PrioritizableRunner& r1, const PrioritizableRunner& r2)
        {
            return r1._pRunner == r2._pRunner && r1._priority == r2._priority;
        }
    };

    ThreadPoolImplBase<Threading::PriorityQueue, PrioritizableRunner> _impl;
};

}
#endif // PRIORITYTHREADPOOL_H
