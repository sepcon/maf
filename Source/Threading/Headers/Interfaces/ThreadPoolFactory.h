#ifndef THREADPOOLFACTORY_H
#define THREADPOOLFACTORY_H

#include "IThreadPool.h"

namespace Threading
{
class ThreadPoolFactory
{
public:
    enum PoolType
    {
        DynamicCount,
        StableCount,
        Priority
    };

    static IThreadPool* createPool(PoolType type, unsigned int poolSize = 0);
};
}
#endif // THREADPOOLFACTORY_H
