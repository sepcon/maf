#ifndef DYNAMICCOUNTHREADPOOL_H
#define DYNAMICCOUNTHREADPOOL_H

#include "headers/Libs/Threading/Interfaces/ThreadJoiner.h"
#include "headers/Libs/Threading/Interfaces/IThreadPool.h"
#include "headers/Libs/Threading/Prv/TP/ThreadPoolImplBase.h"
#include "headers/Libs/Threading/Interfaces/Queue.h"
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
