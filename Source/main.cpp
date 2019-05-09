#include <iostream>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>

#include "Interfaces/ThreadPoolFactory.h"

using namespace std;
using namespace Threading;
using namespace std::chrono;

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

class Calculator : public Threading::Runnable
{
public:
    typedef unsigned long long ValueType;
    Calculator(ValueType from, ValueType to, ValueType& sum)
        : _from(from), _to(to), _sum(sum), _stopped(true)
    {
        _sum = 0;
        setAutoDeleted(true);
    }

    void run() override
    {
        _stopped = false;
        for(ValueType i = _from; i < _to; ++i)
        {
            _sum += i;
            if(_stopped) break;
        }
        gDone++;

        LOGG("Calculator from = " << _from << " to = " << _to << " sum = " << _sum << " stopped!");
    }

    void stop() override
    {
        _stopped = true;
    }

    ~Calculator() override
    {
        if(_sum == 0)
        {
            LOGG("No chance for calculation");
        }
    }
    ValueType _from;
    ValueType _to;
    ValueType& _sum;
    std::atomic_bool _stopped;
    static atomic_int gDone;
};

std::atomic_int Calculator::gDone(1);

#include "Prv/TP/StableThreadPool.h"
void testPool()
{
    auto pool = ThreadPoolFactory::createPool(Threading::PoolType::StableCount, 10);
    pool->setMaxThreadCount(10);
    const Calculator::ValueType MAX = 100000000;
    unsigned int chunkSize = 10000000;
    unsigned int chunkCount = MAX/chunkSize;
    std::vector<Calculator::ValueType> output;
    output.resize(chunkCount + 1);
    auto startTime = system_clock::now();
    for(unsigned int i = 0; i < chunkCount; ++i)
    {
        auto from = chunkSize * i;
        pool->run(new Calculator(from, from + chunkSize, output[i]), i);
    }
    pool->run(new Calculator(chunkCount * chunkSize, MAX + 1, output.back()));

//    while (Calculator::gDone <= output.size())
//    {
//        std::this_thread::sleep_for(std::chrono::milliseconds(50));
//    }
    Calculator::ValueType sum = 0;
    for(auto val : output)
    {
        sum += val;
    }
    LOGG("Sum = " << sum);
    LOGG("DONE, total time: " << duration_cast<microseconds>(system_clock::now() - startTime).count());
//    pool->shutdown();
}

#include "Interfaces/BusyTimer.h"
#include <sstream>

namespace cr = std::chrono;

void scheduleJob(BusyTimer& timer, BusyTimer::JobID id, BusyTimer::Duration duration)
{
    std::cout << id << " starts waiting" << std::endl;
    std::thread{ [&timer, id, duration] {
        auto startTime = cr::system_clock::now();
    timer.start(id, duration, [startTime, duration](BusyTimer::JobID job) {
        std::cout << job << " (" << duration << ") executed after " << cr::duration_cast<cr::milliseconds>(cr::system_clock::now() - startTime).count() << "\n";
    });
    } }.detach();
}

int main()
{
    Threading::BusyTimer timer;
    /*scheduleJob(timer, 1, 5000);
    scheduleJob(timer, 2, 200);
    scheduleJob(timer, 3, 100);
    scheduleJob(timer, 4, 50);
    */
    //start = std::chrono::system_clock::now();

    scheduleJob(timer, 1, 2);
    scheduleJob(timer, 2, 2);
    scheduleJob(timer, 3, 2);
    scheduleJob(timer, 4, 2);
    scheduleJob(timer, 5, 2);
    scheduleJob(timer, 6, 2);
    scheduleJob(timer, 7, 2);
    scheduleJob(timer, 8, 2);
    scheduleJob(timer, 9, 2);

    //scheduleJob(timer, 2, 1000);
    //scheduleJob(timer, 3, 2);
    //scheduleJob(timer, 4, 1500);
    //scheduleJob(timer, 5, 1000);
    //scheduleJob(timer, 6, 2);
    //scheduleJob(timer, 7, 1500);
    //scheduleJob(timer, 8, 1000);
    //scheduleJob(timer, 9, 2);
    //scheduleJob(timer, 10, 1500);
    //scheduleJob(timer, 11, 1000);
    //scheduleJob(timer, 12, 2);

    std::cin.get();
    return 0;
}
