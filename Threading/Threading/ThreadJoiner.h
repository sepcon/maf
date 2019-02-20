#ifndef THREADJOINER_H
#define THREADJOINER_H

namespace Threading {

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
}
#endif // THREADJOINER_H
