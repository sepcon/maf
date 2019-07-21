#include "thaf/threading/Thread.h"
#include "thaf/utils/debugging/Debug.h"

namespace thaf {
namespace threading {

thread_local Thread* Thread::_this = nullptr;

Thread::Thread(Thread &&th) : _thread(std::move(th._thread)){}

Thread& Thread::start()
{
    if(_callable)
    {
        _thread = std::thread(_callable);
    }
    else
    {
        thafErr("Please specify the callback first!");
    }
    return *this;
}

void Thread::join()
{
    _thread.join();
}

void Thread::detach()
{
    _thread.detach();
}

bool Thread::joinable()
{
    return _thread.joinable();
}

void Thread::setSignalHandler(Thread::OnSignalCallback sigHandlerCallback)
{
    _sigHandlerCallback = std::move(sigHandlerCallback);
}

void Thread::regSignals()
{
    signal(SIGABRT, &Thread::onSystemSignal);
    signal(SIGSEGV, &Thread::onSystemSignal);
    signal(SIGILL,  &Thread::onSystemSignal);
    signal(SIGTERM, &Thread::onSystemSignal);
}

void Thread::onSystemSignal(int sig)
{
    if(_this)
    {
        if(_this->_sigHandlerCallback)
        {
            _this->_sigHandlerCallback(sig);
        }
        else
        {
            thafErr("There's no signal handler for thread " << std::this_thread::get_id() << " when signal " << sig << " comes");
        }
    }
}

}
}
