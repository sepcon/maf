#include "PriorityThreadPool.h"
#include  "Queue.h"
namespace Threading
{

class PrioritizableTask
{
    Runable* _pRunner;
    unsigned int _priority;
public:
    PrioritizableTask() {}
    PrioritizableTask(Runable* pRuner, unsigned int priority = 0) :
        _pRunner(pRuner), _priority(priority)
    {
    }

    void run()
    {
        if(_pRunner) _pRunner->run();
    }

    void free()
    {
        if(_pRunner && _pRunner->autoDelete())
        {
            delete _pRunner;
        }
    }
    friend bool operator<(const PrioritizableTask& t1, const PrioritizableTask& t2);
};

bool operator<(const PrioritizableTask &t1, const PrioritizableTask &t2)
{
    return t1._priority < t2._priority;
}

class PriorityThreadPoolImpl
{
public:
    PriorityThreadPoolImpl(size_t threadCount) : _joiner(_pool)
    {
        if(threadCount == 0) threadCount = std::thread::hardware_concurrency();
        for(size_t i = 0; i < threadCount; ++i)
        {
            _pool.emplace_back(std::thread{ &PriorityThreadPoolImpl::threadFunc, this });
        }
    }

    void run(Runable *pRuner, unsigned int priority)
    {
        _taskQueue.push({pRuner, priority});
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
    void threadFunc()
    {
        PrioritizableTask task;
        while(_taskQueue.wait(task))
        {
            task.run();
            task.free();
        }
    }

    PriorityQueue<PrioritizableTask> _taskQueue;
    std::vector<std::thread> _pool;
    ThreadJoiner<decltype (_pool)> _joiner;
};



PriorityThreadPool::PriorityThreadPool(size_t threadCount)
{
    _pImpl = new PriorityThreadPoolImpl(threadCount);
}

PriorityThreadPool::~PriorityThreadPool()
{
    shutdown();
    delete _pImpl;
}

void PriorityThreadPool::run(Runable *pRuner, unsigned int priority)
{
    _pImpl->run(pRuner, priority);
}


unsigned int PriorityThreadPool::activeThreadCount()
{
    return _pImpl->activeThreadCount();
}

void PriorityThreadPool::shutdown()
{
    _pImpl->shutdown();
}


}
