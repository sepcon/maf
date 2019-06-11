#include "headers/Libs/Threading/Interfaces/Thread.h"
#include <iostream>

namespace thaf {
namespace threading {

thread_local Thread* Thread::_this = nullptr;

Thread::Thread(Thread &&th) : _thread(std::move(th._thread)){}

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

void Thread::handleSignal(int sig)
{
    std::cout << "Ignore signal " << sig << std::endl;
}

}
}
