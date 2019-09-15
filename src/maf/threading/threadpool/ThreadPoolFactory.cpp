#include <maf/threading/ThreadPoolFactory.h>
#include "PriorityThreadPool.h"
#include "StableThreadPool.h"
#include "DynamicCountThreadPool.h"

namespace maf {
namespace threading {

std::shared_ptr<IThreadPool> ThreadPoolFactory::createPool(PoolType type, unsigned int poolSize)
{
    std::shared_ptr<IThreadPool> pPool;
    switch (type)
    {
    case PoolType::Priority:
        pPool.reset(new PriorityThreadPool(poolSize));
        break;
    case StableCount:
        pPool.reset(new StableThreadPool(poolSize));
        break;
    case DynamicCount:
        pPool.reset(new VaryCountThreadPool(poolSize));
        break;
    }
    return pPool;
}

}
}
