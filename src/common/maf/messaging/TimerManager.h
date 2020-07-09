#pragma once
#include <functional>
#include <maf/patterns/Patterns.h>
#include <maf/utils/IDManager.h>

namespace maf {
namespace messaging {

class TimerManager : public pattern::Unasignable {
  using IDManager = util::IDManagerT<uint32_t>;

public:
  using JobID = IDManager::IDType;
  typedef long long Duration;
  typedef std::function<void()> TimeOutCallback;
  TimerManager();
  ~TimerManager();
  JobID start(Duration milliseconds, TimeOutCallback callback,
              bool cyclic = false);
  void restart(JobID jid);
  void stop(JobID jid);
  void stop();
  bool isRunning(JobID jid);
  void setCyclic(JobID jid, bool cyclic = true);
  static bool isValid(JobID jid);
  static JobID invalidJobID() { return IDManager::INVALID_ID; }

private:
  struct TimerManagerImpl *pImpl_;
  IDManager idManager_;
};

} // namespace messaging
} // namespace maf
