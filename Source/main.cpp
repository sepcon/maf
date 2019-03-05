#include <iostream>
#include <atomic>
#include <thread>
#include <mutex>

#include "Interfaces/ThreadPoolFactory.h"

using namespace std;
using namespace Threading;

static std::mutex _coutmt;
#define LOGG(exp) \
{ \
std::lock_guard<std::mutex> lock(_coutmt); \
std::cout << exp << std::endl; \
}

class Runner : public Runnable
{
public:
    Runner()
    {
        LOGG( "Thread id: " << std::this_thread::get_id() << " created!" )
        _stopped = true;
    }
    void run() override
    {
        _stopped = false;
        while(!_stopped)
        {
            LOGG( "hello " << std::this_thread::get_id() )
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        LOGG("Thread " << std::this_thread::get_id() << " stopped!")
    }
    void stop() override
    {
        _stopped = true;
    }

    std::atomic_bool _stopped;
};

int main()
{
    auto pool = ThreadPoolFactory::createPool(Threading::PoolType::StableCount);
    for(int i = 0; i < 10; ++i)
    {
        auto p = new Runner;
        p->setAutoDeleted(true);
        pool->run(p);
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    pool->shutdown();
    delete pool;
    LOGG("DONE")
    return 0;
}
