#ifndef THREADSAFEQUEUE_H
#define THREADSAFEQUEUE_H

#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>

// mutex auto lock
#define MT_ALOCK_NUM_SUB(mt, line) std::lock_guard<std::mutex> lock##line(mt)
#define MT_ALOCK_NUM(mt, line) MT_ALOCK_NUM_SUB(mt, line)
#define MT_ALOCK(mt) MT_ALOCK_NUM(mt, __LINE__)

namespace thaf {
namespace threading {

template<class Impl>
class ThreadSafeQueue
{
public:
    using reference = typename Impl::reference;
    using const_reference = typename Impl::const_reference;
    using value_type = typename Impl::value_type;
    using ApplyAction = std::function<void(value_type&)>;

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
    void push(const value_type& data)
    {
        if(!isClosed())
        {
            MT_ALOCK(_mt);
            _queue.push(data);
            _condVar.notify_all();
        }
    }
    void push(value_type&& data)
    {
        if(!isClosed())
        {
            MT_ALOCK(_mt);
            _queue.emplace(data);
            _condVar.notify_all();
        }
    }

    bool wait(value_type& value)
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
        if(!isClosed())
        {
            _closed.store(true, std::memory_order_release);
            _condVar.notify_all();
        }
    }

    bool isClosed() const { return _closed.load(std::memory_order_acquire); }

    void clear(ApplyAction onClearCallback = nullptr)
    {
        MT_ALOCK(_mt);
        while(!_queue.empty())
        {
            value_type v = _queue.front();
            if( onClearCallback ) onClearCallback(v);
            _queue.pop();
        }
    }

    size_t size()
    {
        MT_ALOCK(_mt);
        return _queue.size();
    }

private:
    Impl _queue;
    std::mutex _mt;
    std::condition_variable _condVar;
    std::atomic_bool _closed;
};

}

namespace stdwrap
{
template <typename T> using Queue = std::queue<T>;

template <typename T, typename Comp>
class PriorityQueue
        : public std::priority_queue<T, std::vector<T>, Comp>
{
public:
    using __Base = std::priority_queue<T, std::vector<T>, Comp>;
    using std::priority_queue<T, std::vector<T>, Comp>::priority_queue;
    using reference = typename __Base::reference;
    using const_reference = typename __Base::const_reference;
    using value_type = typename __Base::value_type;

    const_reference front() const
    {
        return __Base::top();
    }
};
}
}

#undef MT_ALOCK
#undef MT_ALOCK_NUM
#undef MT_ALOCK_NUM_SUB
#endif // THREADSAFEQUEUE_H
