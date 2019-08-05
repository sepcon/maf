#ifndef THREADPOOLFACTORY_H
#define THREADPOOLFACTORY_H

#include "IThreadPool.h"
#include <memory>

namespace maf {
namespace threading {

enum PoolType
{
    DynamicCount,
    StableCount,
    Priority
};

class ThreadPoolFactory
{
public:    
    static std::shared_ptr<IThreadPool> createPool(PoolType type, unsigned int poolSize = 0);
};
}
}
#endif // THREADPOOLFACTORY_H
