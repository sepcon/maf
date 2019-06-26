#ifndef DYNAMICCOUNTHREADPOOL_H
#define DYNAMICCOUNTHREADPOOL_H

#include "thaf/Threading/ThreadJoiner.h"
#include "thaf/Threading/IThreadPool.h"
#include "thaf/Threading/Prv/TP/ThreadPoolImplBase.h"
#include "thaf/Threading/Queue.h"
#include <thread>

namespace thaf {
namespace threading
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
}

#endif // DYNAMICCOUNTHREADPOOL_H
