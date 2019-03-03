#ifndef VARYCOUNTHREADPOOL_H
#define VARYCOUNTHREADPOOL_H

#include "Interfaces/ThreadJoiner.h"
#include "Interfaces/IThreadPool.h"
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
    /**
      * copt: call on pool's threads
      */
    void coptExecuteTasks(Runnable* pRuner);
    void coptRunPendingTask();

    Queue<Runnable*> _taskQueue;
    std::vector<std::thread> _pool;
    ThreadJoiner<decltype (_pool)> _threadJoiner;
    std::atomic_uint _maxThreadCount;
};
}


#endif // VARYCOUNTHREADPOOL_H
