#ifndef THREADPOOLFACTORY_H
#define THREADPOOLFACTORY_H

#include "IThreadPool.h"

namespace Threading
{
enum PoolType
{
    DynamicCount,
    StableCount,
    Priority
};

class ThreadPoolFactory
{
public:    
    static IThreadPool* createPool(PoolType type, unsigned int poolSize = 0);
};
}
#endif // THREADPOOLFACTORY_H
