#include "DynamicCountThreadPool.h"

namespace maf {
namespace threading {

VaryCountThreadPool::VaryCountThreadPool(unsigned int nThreadCount):
    _impl{ nThreadCount, &threading::run, &threading::stop, &threading::done}
{
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
