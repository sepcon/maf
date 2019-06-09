#include "headers/Threading/Prv/TP/DynamicCountThreadPool.h"
#include <iostream>
#include <string>
#include <sstream>

namespace thaf {
namespace Threading {

VaryCountThreadPool::VaryCountThreadPool(unsigned int nThreadCount):
    _impl{ nThreadCount, &Threading::run, &Threading::stop, &Threading::done}
{
    std::cout << "Pool created with max thread count = " << _impl.maxThreadCount() << std::endl;
}

void VaryCountThreadPool::run(Runnable *pRuner, unsigned int /*priority*/)
{
    if(pRuner)
    {
        if(_impl.activeThreadCount() < _impl.maxThreadCount())
        {
            _impl.tryLaunchNewThread();
        }
        _impl.run(pRuner);
    }
}

void VaryCountThreadPool::setMaxThreadCount(unsigned int nThreadCount)
{
    _impl.setMaxThreadCount(nThreadCount);
}

unsigned int VaryCountThreadPool::activeThreadCount()
{
    return _impl.activeThreadCount();
}

void VaryCountThreadPool::shutdown()
{
    _impl.shutdown();
}


VaryCountThreadPool::~VaryCountThreadPool()
{
}

}
}
