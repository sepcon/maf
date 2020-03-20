#ifndef STABLETHREADPOOL_H
#define STABLETHREADPOOL_H

#include <maf/threading/IThreadPool.h>
#include <maf/threading/Queue.h>

namespace maf {
namespace threading {
class StableThreadPool : public IThreadPool {
public:
  StableThreadPool(unsigned int threadCount = 0);
  ~StableThreadPool() override;
  virtual void run(Runnable *pRuner, unsigned int priority = 0) override;
  virtual void setMaxThreadCount(unsigned int nThreadCount) override;
  virtual unsigned int activeThreadCount() override;
  virtual void shutdown() override;

private:
  struct __I *_pI;
};

} // namespace threading
} // namespace maf
#endif // STABLETHREADPOOL_H
