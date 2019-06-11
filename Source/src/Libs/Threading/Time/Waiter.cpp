#include "headers/Libs/Threading/Interfaces/Waiter.h"
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>

using namespace std::chrono;

namespace thaf {
namespace threading {

class WaiterImpl
{
    std::condition_variable _condVar;
    std::mutex _mutex;
    std::atomic_bool _stopped;
    system_clock::time_point _when;
    Waiter::CallbackType _callback;

    void threadFunc()
    {
        while(!_stopped.load())
        {
            std::unique_lock<std::mutex> _lock(_mutex);
            if(_when > system_clock::now())
            {
                _condVar.wait_until(_lock, _when);
            }
            if(_stopped.load())
            {
                break;
            }
            else if(_when <= system_clock::now())
            {
                _stopped.store(true);
                _callback();
                break;
            }
        }
    }
    void setCallback(Waiter::CallbackType callback)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _callback = callback;
    }
    void setWhen(system_clock::time_point when)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _when = when;
    }
    void start()
    {
        if(isRunning())
        {
            restart();
        }
        else
        {
            _stopped.store(false);
            std::thread{ &WaiterImpl::threadFunc, this }.detach();
        }
    }
    bool restart()
    {
        if(isRunning())
        {
            _condVar.notify_one();
            return true;
        }
        else
        {
            return false;
        }
    }
public:
    WaiterImpl() : _stopped(true){}

    void waitFor(milliseconds ms, Waiter::CallbackType callback)
    {
        waitUtil(system_clock::now() + duration_cast<system_clock::duration>(ms), callback);
    }

    void waitUtil(system_clock::time_point when, Waiter::CallbackType callback)
    {
        setCallback(callback);
        setWhen(when);
        start();
    }

    void stop()
    {
        if(isRunning())
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _stopped.store(true);
            _condVar.notify_one();
        }
    }

    bool isRunning()
    {
        return !_stopped.load();
    }
};

Waiter::Waiter()
{
    _pImpl = new WaiterImpl;
}

Waiter::~Waiter()
{
    _pImpl->stop();
    delete _pImpl;
}

void Waiter::waitUtil(std::chrono::system_clock::time_point when, CallbackType callback)
{
    _pImpl->waitUtil(when, callback);
}

void Waiter::waitFor(milliseconds ms, CallbackType callback)
{
    _pImpl->waitFor(ms, callback);
}

void Waiter::stop()
{
    _pImpl->stop();
}

bool Waiter::isRunning()
{
    return _pImpl->isRunning();
}

}
}
