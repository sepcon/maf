#include "Interfaces/Signal.h"
#include <condition_variable>
#include <mutex>
#include <atomic>

class SignalImpl
{
public:
    SignalImpl() : _stopped(false){}
    ~SignalImpl()
    {

    }
    void wait()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _condVar.wait(lock);
    }

    void notify()
    {
        _condVar.notify_one();
    }

    void notifyAll()
    {
        _condVar.notify_all();
    }

    void stop()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _stopped.store(true, std::memory_order_relaxed);
        _condVar.notify_all();
    }

    void lock()
    {
        _mutex.lock();
    }
    void unlock()
    {
        _mutex.unlock();
    }
    bool isStopped() const
    {
        return _stopped.load();
    }
private:
    std::condition_variable _condVar;
    std::mutex _mutex;
    std::atomic_bool _stopped;
};

Signal::Signal()
{
    _pImpl = new SignalImpl;
}

Signal::~Signal()
{
    delete _pImpl;
}

void Signal::wait()
{
    _pImpl->wait();
}

void Signal::notify()
{
    _pImpl->notify();
}

void Signal::notifyAll()
{
    _pImpl->notifyAll();
}

void Signal::stop()
{
    _pImpl->stop();
}

void Signal::lock()
{
    _pImpl->lock();
}

void Signal::unlock()
{
    _pImpl->unlock();
}

bool Signal::isStopped() const
{
    return _pImpl->isStopped();
}
