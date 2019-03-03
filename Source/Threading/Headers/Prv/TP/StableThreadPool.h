#ifndef STABLETHREADPOOL_H
#define STABLETHREADPOOL_H

#include "Interfaces/IThreadPool.h"
#include "Interfaces/Queue.h"
#include "ThreadPoolImplBase.h"

namespace Threading
{
class StableThreadPool : public IThreadPool
{
public:
    StableThreadPool(unsigned int threadCount = 0);
    ~StableThreadPool() override;
    virtual void run(Runnable* pRuner, unsigned int priority = 0) override;
    virtual void setMaxThreadCount(unsigned int /*nThreadCount*/) override {} // Cannot tune the thread count because it is stable
    virtual unsigned int activeThreadCount() override;
    virtual void shutdown() override;

private:
    struct RunnerRef
    {
        Runnable* _pRunner;
        RunnerRef(Runnable* pRunner = nullptr) : _pRunner(pRunner)
        {
        }
        void run() { if(_pRunner) _pRunner->run(); }
        void stop() { if(_pRunner) _pRunner->stop(); }
    };
    ThreadPoolImplBase<Queue, RunnerRef> _impl;
};

}



#endif // STABLETHREADPOOL_H
