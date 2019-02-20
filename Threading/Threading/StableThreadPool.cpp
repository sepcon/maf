#include <thread>
#include <vector>
#include "TheadSafeQueue.h"
#include "StableThreadPool.h"
#include "ThreadJoiner.h"




namespace Threading
{

class StableThreadPoolImpl
{
    std::vector<std::thread> _pool;
    Threading::TheadSafeQueue<Runable*> _taskQueue;
    ThreadJoiner<decltype (_pool)> _threadJoiner;
public:
    StableThreadPoolImpl(size_t maxCount = 0): _threadJoiner(_pool)
    {
        if(maxCount == 0)
        {
            maxCount = std::thread::hardware_concurrency();
        }
        for(size_t i = 0; i < maxCount; ++i)
        {
            _pool.emplace_back(std::thread{&StableThreadPoolImpl::coptRunPendingTask, this});
        }
    }
    ~StableThreadPoolImpl()
    {
        _taskQueue.close();
    }
    void run(Runable* pRuner)
    {
        _taskQueue.push(pRuner);
    }

    unsigned int activeThreadCount()
    {
        return static_cast<unsigned int>(_pool.size());
    }

    void shutdown()
    {
        _taskQueue.close();
    }

private:
    void coptRunPendingTask()
    {
        Runable* pRuner;
        while(_taskQueue.wait(pRuner))
        {
            pRuner->run();
        }
    }

};

StableThreadPool::StableThreadPool(unsigned int threadCount)
{
    _pImpl = new StableThreadPoolImpl(threadCount);
}

StableThreadPool::~StableThreadPool()
{
    delete _pImpl;
}

void StableThreadPool::run(Runable *pRuner, int /*priority*/)
{
    _pImpl->run(pRuner);
}

unsigned int StableThreadPool::activeThreadCount()
{
    return _pImpl->activeThreadCount();
}

void StableThreadPool::shutdown()
{
    _pImpl->shutdown();
}

}
