#include <iostream>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>

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

        LOGG("Calculator " << _sum << " stopped!");
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

int main()
{
    auto pool = ThreadPoolFactory::createPool(Threading::PoolType::Priority);
    pool->setMaxThreadCount(10);
    const Calculator::ValueType MAX = 1000000000;
    unsigned int chunkSize = 100000000;
    unsigned int chunkCount = MAX/chunkSize;
    std::vector<Calculator::ValueType> output;
    output.resize(chunkCount + 1);
    for(unsigned int i = 0; i < chunkCount; ++i)
    {
        auto from = chunkSize * i;
        pool->run(new Calculator(from, from + chunkSize, output[i]), i);
    }
    pool->run(new Calculator(chunkCount * chunkSize, MAX + 1, output.back()));

    while (Calculator::gDone <= output.size())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    Calculator::ValueType sum = 0;
    for(auto val : output)
    {
        sum += val;
    }
    LOGG("Sum = " << sum);
    LOGG("DONE")
    return 0;
}
