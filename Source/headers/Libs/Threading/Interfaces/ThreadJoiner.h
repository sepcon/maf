#ifndef THREADJOINER_H
#define THREADJOINER_H

#include <thread>

namespace thaf {
namespace threading {

template <class ThreadContainer>
class ThreadJoiner
{
    ThreadContainer& _threads;
public:
    ThreadJoiner(ThreadContainer& threads): _threads(threads){}
    ~ThreadJoiner()
    {
        for(auto& th : _threads)
        {
            if(th.joinable())
            {
                th.join();
            }
        }
    }
};

template<>
class ThreadJoiner<std::thread>
{
    std::thread& _th;
public:
    ThreadJoiner(std::thread& th) : _th(th) {}
    ~ThreadJoiner()
    {
        if(_th.joinable()) _th.join();
    }
};

}
}
#endif // THREADJOINER_H
