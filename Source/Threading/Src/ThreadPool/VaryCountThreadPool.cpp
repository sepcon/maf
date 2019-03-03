#include "Prv/TP/VaryCountThreadPool.h"
#include <iostream>
#include <string>
#include <sstream>

namespace Threading
{

VaryCountThreadPool::VaryCountThreadPool(unsigned int nThreadCount) : _threadJoiner(_pool)
{
    if(0 == nThreadCount)
    {
        _maxThreadCount.store(std::thread::hardware_concurrency());
    }
    else
    {
        _maxThreadCount.store(nThreadCount);
    }
    std::cout << "Pool created with max thread count = " << _maxThreadCount << std::endl;
}

void VaryCountThreadPool::run(Runnable *pRuner, unsigned int /*priority*/)
{
    if(pRuner)
    {
        //when taskqueue is not empty, means that all the active threads are busy -> then create new thread if possible
        if( (_pool.size() < _maxThreadCount.load()) && !_taskQueue.empty())
        {
            std::cout << "Created thread " << std::this_thread::get_id() << " for new task" << std::endl;
            _pool.emplace_back(std::thread{&VaryCountThreadPool::coptExecuteTasks, this, pRuner});
        }
        else
        {
            _taskQueue.push(pRuner);
        }
    }
}

void VaryCountThreadPool::setMaxThreadCount(unsigned int nThreadCount)
{
    _maxThreadCount.store(nThreadCount);
}

unsigned int VaryCountThreadPool::activeThreadCount()
{
    return static_cast<unsigned int>(_pool.size());
}

void VaryCountThreadPool::shutdown()
{
    _taskQueue.close();
}


VaryCountThreadPool::~VaryCountThreadPool()
{
    shutdown();
}

void VaryCountThreadPool::coptExecuteTasks(Runnable *pRuner)
{
    if(pRuner)
    {
        pRuner->run();
    }
    coptRunPendingTask();
}

void VaryCountThreadPool::coptRunPendingTask()
{
    Runnable* pRuner;
    while(_taskQueue.wait(pRuner))
    {
        pRuner->run();
        if(pRuner->autoDelete())
        {
            delete pRuner;
        }
    }
}

}
