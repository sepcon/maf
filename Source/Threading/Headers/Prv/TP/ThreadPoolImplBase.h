#ifndef THREADPOOLIMPLBASE_H
#define THREADPOOLIMPLBASE_H

#include "Interfaces/IThreadPool.h"
#include "Interfaces/ThreadJoiner.h"
#include "Interfaces/Runnable.h"
#include <thread>
#include <vector>
#include <list>
#include <mutex>
#include <atomic>
#include <functional>

namespace Threading {

/*! \brief The base class used for implement the variants of threadpool
 * class ThreadPoolImplBase
 *
 */

template< class TaskQueue>
class ThreadPoolImplBase
{
    typedef typename TaskQueue::value_type Task;
    typedef typename TaskQueue::reference TaskRef;
    typedef typename TaskQueue::const_reference TaskCRef;
    typedef std::function<void (TaskRef)> TaskExc;

    std::vector<std::thread> _pool;
    TaskQueue _taskQueue;
    std::once_flag _shutdowned;
    TaskExc _fRun;
    TaskExc _fStop;
    TaskExc _fDone;

    struct ActiveRunners
    {
        std::list<Task> _runnings;
        std::mutex _mutex;
        TaskExc _fStop;
        void add(Task task)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _runnings.push_back(task);
        }
        void remove(Task task)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            for(auto iTask = _runnings.begin(); iTask != _runnings.end(); ++iTask)
            {
                if(*iTask == task)
                {
                    _runnings.erase(iTask);
                    break;
                }
            }
        }
        void stopAll()
        {
            std::lock_guard<std::mutex> lock(_mutex);
            for(auto& task : _runnings)
            {
                _fStop(task);
            }
        }
    } _activeRunners;

    void waitForThreadsExited()
    {
        for(auto& th : _pool)
        {
            if(th.joinable()) { th.join(); }
        }
    }
public:
    ThreadPoolImplBase(size_t maxCount, TaskExc fRun, TaskExc fStop, TaskExc fDone):
        _fRun(fRun),
        _fStop(fStop),
        _fDone(fDone)
    {
        _activeRunners._fStop = _fStop;
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
        shutdown();
    }

    void run(Task task)
    {
        _taskQueue.push(task);
    }

    unsigned int activeThreadCount()
    {
        return static_cast<unsigned int>(_pool.size());
    }

    void shutdown()
    {
        std::call_once(_shutdowned, &ThreadPoolImplBase::stopThePool, this);
    }

    void stopThePool()
    {
        _taskQueue.close();
        _activeRunners.stopAll();
        _taskQueue.clear(_fDone);
        waitForThreadsExited();
    }

private:
    void coptRun(Task task)
    {
        _activeRunners.add(task);
        _fRun(task);
        _activeRunners.remove(task);
    }
    void coptRunPendingTask()
    {
        Task task;
        while(_taskQueue.wait(task))
        {
            coptRun(task);
            _fDone(task);
        }
    }

};
}

#endif // THREADPOOLIMPLBASE_H
