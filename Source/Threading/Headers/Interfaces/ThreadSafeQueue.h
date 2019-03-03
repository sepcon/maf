#ifndef THREADSAFEQUEUE_H
#define THREADSAFEQUEUE_H

#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>

// mutex auto lock
#define MT_ALOCK_NUM_SUB(mt, line) std::lock_guard<std::mutex> lock##line(mt)
#define MT_ALOCK_NUM(mt, line) MT_ALOCK_NUM_SUB(mt, line)
#define MT_ALOCK(mt) MT_ALOCK_NUM(mt, __LINE__)

namespace Threading
{

template<typename T, class Queue>
class ThreadSafeQueue
{
public:
    ThreadSafeQueue() : _closed(false)
    {
    }
    ~ThreadSafeQueue()
    {
        close();
    }
    bool empty()
    {
        MT_ALOCK(_mt);
        return _queue.empty();
    }
    void push(const T& data)
    {
        if(!isClosed())
        {
            MT_ALOCK(_mt);
            _queue.push(data);
            _condVar.notify_all();
        }
    }
    void push(T&& data)
    {
        if(!isClosed())
        {
            MT_ALOCK(_mt);
            _queue.emplace(data);
            _condVar.notify_all();
        }
    }

    bool wait(T& value)
    {
        std::unique_lock<std::mutex> lock(_mt);
        while(!isClosed() && _queue.empty())
        {
            _condVar.wait(lock);
        }
        if(!isClosed())
        {
            value = _queue.front();
            _queue.pop();
            return true;
        }
        else
        {
            return false;
        }
    }

    void close()
    {
        MT_ALOCK(_mt);
        _closed.store(true, std::memory_order_relaxed);
        _condVar.notify_all();
        _queue = {};
    }

    bool isClosed() const { return _closed.load(); }

    Queue&& takeAndClearQueue()
    {
        return std::move(_queue);
    }

    size_t size()
    {
        MT_ALOCK(_mt);
        return _queue.size();
    }

private:
    Queue _queue;
    std::mutex _mt;
    std::condition_variable _condVar;
    std::atomic_bool _closed;
};

}

namespace stdwrap
{
template <typename T> using Queue = std::queue<T>;
template <typename T> class PriorityQueue
        : public std::priority_queue<T>
{
public:
    using __Base = std::priority_queue<T>;
    using __Base::priority_queue;
    typename __Base::const_reference front() const
    {
        return __Base::top();
    }
};
}

#undef MT_ALOCK
#undef MT_ALOCK_NUM
#undef MT_ALOCK_NUM_SUB
#endif // THREADSAFEQUEUE_H
