#include <maf/threading/Thread.h>
#include <maf/logging/Logger.h>
#include <signal.h>
#include <cassert>


namespace maf { using logging::Logger;
namespace threading {

static thread_local Thread::OnSignalCallback _tlSigHandlerCallback = nullptr;

Thread::Thread(Thread &&th)
{
    assert(!th.joinable() && "Not allow moving thread that already running!");
    takeFrom(std::move(th));
}

Thread &Thread::operator=(Thread &&th)
{
    takeFrom(std::move(th));
    return *this;
}

Thread &Thread::start()
{
    if(_callable)
    {
        auto threadFunc = [ callable = std::move(_callable),
                         sigHandler = std::move(_sigHandlerCallback)] () mutable
        {
            _tlSigHandlerCallback = std::move(sigHandler);
            regSignals();
            callable();
        };
        _thread = std::thread(threadFunc);
    }
    else
    {
        Logger::error("Please specify the callback first!");
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


Thread::~Thread()
{
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
    if(_tlSigHandlerCallback)
    {
        _tlSigHandlerCallback(sig);
    }
    else
    {
        Logger::error(
            "There's no signal handler for thread ",
            std::this_thread::get_id() ,
            " when signal " ,
            sig ,
            " comes"
            );
    }
}

void Thread::takeFrom(Thread &&th)
{
    if(this != &th)
    {
        _thread = std::move(th._thread);
        _callable = std::move(th._callable);
        _sigHandlerCallback = std::move(th._sigHandlerCallback);
    }
}

}
}
