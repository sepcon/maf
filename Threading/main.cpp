#include <iostream>
#include "Threading/TheadSafeQueue.h"
#include<thread>
#include <numeric>
#include <chrono>
#include <sstream>
#include <future>

using namespace std;

#include "Threading/TheadSafeQueue.h"
#include "Threading/VaryCountThreadPool.h"
#include "Threading/StableThreadPool.h"



class Sumer : public Threading::Runable
{
public:
    Sumer(std::vector<unsigned long long>::iterator start, std::vector<unsigned long long>::iterator end)
        : _start(start), _end(end)
    {
        setAutoDeleted(true);
        gSumResults.emplace_back(_p.get_future());
    }

    void run() override
    {
        unsigned long long res = 0;
        for(auto it = _start; it < _end; ++it)
        {
            res += *it;
        }
        _p.set_value(res);
    }

    std::vector<unsigned long long>::iterator _start;
    std::vector<unsigned long long>::iterator _end;
    std::promise<unsigned long long> _p;
    static std::vector<std::future<unsigned long long> > gSumResults;
};

std::vector<std::future<unsigned long long> > Sumer::gSumResults;

int main()
{
    Threading::StableThreadPool pool(0);
    std::vector<unsigned long long> v;
    const unsigned long long max = 100000000;
    const unsigned long long chunkSize = 1000000;
    for(unsigned long long i = 1; i <= max; ++i)
    {
        v.push_back(i);
    }


    auto startTime = std::chrono::system_clock::now();
    unsigned long long numOfTasks = max / chunkSize;
    std::vector<unsigned long long>::iterator start = v.begin();
    std::vector<unsigned long long>::iterator end = start;
//    Sumer sumer(v.begin(), v.end());
//    sumer.run();
//    unsigned long long sum = Sumer::gSumResults[0].get();
    for(unsigned long long i = 0; i < numOfTasks; ++i)
    {
        start = end;
        std::advance(end, chunkSize);
        pool.run(new Sumer(start, end));
    }

    pool.run(new Sumer(end, v.end()));

    std::cout << "pool size now: = " << pool.activeThreadCount() << std::endl;
    unsigned long long sum = 0;
    for(auto& f : Sumer::gSumResults)
    {
        unsigned long long ret = f.get();
        sum += ret;
        std::cout << "one more chunk sum done! with value: " << ret << std::endl;
    }

    std::cout << "Sum of 1 to " << max << " = " << sum << std::endl;
    std::cout << "Total execution time = " << (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime)).count() << std::endl;
    return 0;
}
