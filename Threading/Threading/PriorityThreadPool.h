#ifndef PRIORITYTHREADPOOL_H
#define PRIORITYTHREADPOOL_H

#include "IThreadPool.h"
#include "ThreadJoiner.h"
#include <vector>
#include <thread>

namespace Threading
{


class PriorityThreadPool : public IThreadPool
{
public:
    PriorityThreadPool(size_t threadCount = 0);
    ~PriorityThreadPool();
    virtual void run(Runable* pRuner, unsigned int priority = 0) override;
    virtual void setMaxThreadCount(unsigned int /*nThreadCount*/) override {}
    virtual unsigned int activeThreadCount() override;
    virtual void shutdown() override;
private:
    class PriorityThreadPoolImpl * _pImpl;
};

}
#endif // PRIORITYTHREADPOOL_H
