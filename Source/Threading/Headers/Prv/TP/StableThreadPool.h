#ifndef STABLETHREADPOOL_H
#define STABLETHREADPOOL_H

#include "Interfaces/IThreadPool.h"
#include "Interfaces/Queue.h"

namespace Threading
{
class StableThreadPool : public IThreadPool
{
public:
    StableThreadPool(unsigned int threadCount = 0);
    ~StableThreadPool() override;
    virtual void run(Runnable* pRuner, unsigned int priority = 0) override;
    virtual void setMaxThreadCount(unsigned int nThreadCount) override;
    virtual unsigned int activeThreadCount() override;
    virtual void shutdown() override;

private:
    struct __I* _pI;
};

}



#endif // STABLETHREADPOOL_H
