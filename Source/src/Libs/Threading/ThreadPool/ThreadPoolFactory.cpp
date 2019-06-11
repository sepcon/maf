#include "headers/Libs/Threading/Interfaces/ThreadPoolFactory.h"
#include "headers/Libs/Threading/Prv/TP/PriorityThreadPool.h"
#include "headers/Libs/Threading/Prv/TP/StableThreadPool.h"
#include "headers/Libs/Threading/Prv/TP/DynamicCountThreadPool.h"

namespace thaf {
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
