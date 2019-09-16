#ifndef DYNAMICCOUNTHREADPOOL_H
#define DYNAMICCOUNTHREADPOOL_H

#include <maf/threading/IThreadPool.h>
#include <maf/threading/Queue.h>
#include <maf/threading/ThreadPoolImplBase.h>

namespace maf {
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
