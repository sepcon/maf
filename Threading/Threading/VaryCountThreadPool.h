#ifndef VARYCOUNTHREADPOOL_H
#define VARYCOUNTHREADPOOL_H

#include <thread>
#include "ThreadJoiner.h"
#include "IThreadPool.h"
#include "TheadSafeQueue.h"

namespace Threading
{
class VaryCountThreadPool : public IThreadPool
{
public:
    VaryCountThreadPool(unsigned int nThreadCount = 0);
    virtual void run(Runable* pRuner, int priority = 0) override;
    virtual void setMaxThreadCount(unsigned int nThreadCount) override;
    virtual unsigned int activeThreadCount() override;
    virtual void shutdown() override;
    ~VaryCountThreadPool() override;

private:
    /**
      * copt: call on pool's threads
      */
    void coptExecuteTasks(Runable* pRuner);
    void coptRunPendingTask();

    TheadSafeQueue<Runable*> _taskQueue;
    std::vector<std::thread> _pool;
    ThreadJoiner<decltype (_pool)> _threadJoiner;
    std::atomic_uint _maxThreadCount;
};
}


#endif // VARYCOUNTHREADPOOL_H
