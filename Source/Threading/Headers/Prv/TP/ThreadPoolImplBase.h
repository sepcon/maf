#ifndef THREADPOOLIMPLBASE_H
#define THREADPOOLIMPLBASE_H

#include "Interfaces/IThreadPool.h"
#include "Interfaces/ThreadJoiner.h"
#include "Interfaces/Runnable.h"
#include <thread>
#include <vector>
#include <list>
#include <mutex>

namespace Threading {

/*! \brief The base class used for implement the variants of threadpool
 * class ThreadPoolImplBase
 *
 */



template< template<class> class QueueType, typename RunnerType>
class ThreadPoolImplBase
{
    std::vector<std::thread> _pool;
    QueueType<RunnerType>  _taskQueue;
    ThreadJoiner<decltype (_pool)> _threadJoiner;
    struct ActiveRunners
    {
        std::list<RunnerType> _runnings;
        std::mutex _mutex;
        void add(RunnerType pRunner)
        {
            if(pRunner)
            {
                std::lock_guard<std::mutex> lock(_mutex);
                _runnings.push_back(pRunner);
            }
        }
        void remove(RunnerType pRunner)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            for(auto iRunner = _runnings.begin(); iRunner != _runnings.end(); ++iRunner)
            {
                if(*iRunner == pRunner)
                {
                    _runnings.erase(pRunner);
                    break;
                }
            }
        }
        void stopAll()
        {
            std::lock_guard<std::mutex> lock(_mutex);
            for(auto& pRunner : _runnings)
            {
                pRunner.stop();
            }
        }
    } _activeRunners;

public:
    ThreadPoolImplBase(size_t maxCount = 0): _threadJoiner(_pool)
    {
        if(maxCount == 0)
        {
            maxCount = std::thread::hardware_concurrency();
        }
        for(size_t i = 0; i < maxCount; ++i)
        {
            _pool.emplace_back(std::thread{&ThreadPoolImplBase::coptRunPendingTask, this});
        }
    }
    ~ThreadPoolImplBase()
    {
        _taskQueue.close();
    }
    void run(RunnerType pRuner)
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
        _activeRunners.stopAll();
    }

private:
    void coptRun(RunnerType pRuner)
    {
        if(pRuner)
        {
            _activeRunners.add(pRuner);
            pRuner.run();
            _activeRunners.remove(pRuner);
        }
    }
    void coptRunPendingTask()
    {
        RunnerType pRuner;
        while(_taskQueue.wait(pRuner))
        {
            pRuner.run();
        }
    }

};
}

#endif // THREADPOOLIMPLBASE_H
