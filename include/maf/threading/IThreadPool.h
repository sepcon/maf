#ifndef ITHREADPOOL_H
#define ITHREADPOOL_H

#include "Runnable.h"

namespace maf {
namespace threading {

class IThreadPool {
public:
  virtual void run(Runnable *pRuner, unsigned int priority = 0) = 0;
  virtual void setMaxThreadCount(unsigned int nThreadCount) = 0;
  virtual unsigned int activeThreadCount() = 0;
  virtual void shutdown() = 0;
  virtual ~IThreadPool() {}
};

} // namespace threading
} // namespace maf

#endif // ITHREADPOOL_H
