#ifndef STABLETHREADPOOL_H
#define STABLETHREADPOOL_H

#include "IThreadPool.h"


namespace Threading
{
class StableThreadPool : public IThreadPool
{
public:
    StableThreadPool(unsigned int threadCount = 0);
    ~StableThreadPool() override;
    virtual void run(Runable* pRuner, unsigned int priority = 0) override;
    virtual void setMaxThreadCount(unsigned int /*nThreadCount*/) override {} // Cannot tune the thread count because it is stable
    virtual unsigned int activeThreadCount() override;
    virtual void shutdown() override;

private:
    class StableThreadPoolImpl* _pImpl;
};

}



#endif // STABLETHREADPOOL_H
