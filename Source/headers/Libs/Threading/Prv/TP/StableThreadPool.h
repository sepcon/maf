#ifndef STABLETHREADPOOL_H
#define STABLETHREADPOOL_H

#include "headers/Libs/Threading/Interfaces/IThreadPool.h"
#include "headers/Libs/Threading/Interfaces/Queue.h"

namespace thaf {
namespace threading {
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
}
#endif // STABLETHREADPOOL_H
