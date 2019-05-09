#ifndef DYNAMICCOUNTHREADPOOL_H
#define DYNAMICCOUNTHREADPOOL_H

#include "Interfaces/ThreadJoiner.h"
#include "Interfaces/IThreadPool.h"
#include "ThreadPoolImplBase.h"
#include "Interfaces/Queue.h"
#include <thread>

namespace Threading
{
class VaryCountThreadPool : public IThreadPool
{
public:
    VaryCountThreadPool(unsigned int nThreadCount = 0);
    virtual void run(Runnable* pRuner, unsigned int priority = 0) override;
    virtual void setMaxThreadCount(unsigned int nThreadCount) override;
    virtual unsigned int activeThreadCount() override;
    virtual void shutdown() override;
    ~VaryCountThreadPool() override;

private:
    ThreadPoolImplBase<Queue<Runnable*>> _impl;
};
}


#endif // DYNAMICCOUNTHREADPOOL_H
