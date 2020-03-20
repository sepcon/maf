#ifndef THREADPOOLFACTORY_H
#define THREADPOOLFACTORY_H

#include "IThreadPool.h"
#include <maf/export/MafExport_global.h>
#include <memory>

namespace maf {
namespace threading {

enum PoolType { DynamicCount, StableCount, Priority };

class ThreadPoolFactory {
public:
  MAF_EXPORT static std::shared_ptr<IThreadPool>
  createPool(PoolType type, unsigned int poolSize = 0);
};
} // namespace threading
} // namespace maf
#endif // THREADPOOLFACTORY_H
