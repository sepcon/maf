#include "Interfaces/ThreadPoolFactory.h"
#include "Prv/TP/PriorityThreadPool.h"
#include "Prv/TP/StableThreadPool.h"
#include "Prv/TP/VaryCountThreadPool.h"

namespace Threading
{

IThreadPool *ThreadPoolFactory::createPool(PoolType type, unsigned int poolSize)
{
    IThreadPool* pPool = nullptr;
    switch (type)
    {
    case PoolType::Priority:
        pPool = new PriorityThreadPool(poolSize);
        break;
    case StableCount:
        pPool = new StableThreadPool(poolSize);
        break;
    case DynamicCount:
        pPool = new VaryCountThreadPool(poolSize);
        break;
    }
    return pPool;
}

}
