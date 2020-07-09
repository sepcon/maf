#include "TimerManager.h"
#include "TimerManagerImpl.h"
#include <maf/logging/Logger.h>

namespace maf {

namespace messaging {

TimerManager::TimerManager() { pImpl_ = new TimerManagerImpl; }

TimerManager::~TimerManager() {
  if (pImpl_) {
    try {
      pImpl_->stop();
      delete pImpl_;
    } catch (const std::exception &e) {
      MAF_LOGGER_WARN("Caught exception: ", e.what());
    } catch (...) {
      MAF_LOGGER_WARN("Uncaught exception!");
    }
  }
}

void TimerManager::restart(JobID jid) {
  if (isValid(jid)) {
    pImpl_->restart(jid);
  }
}

TimerManager::JobID TimerManager::start(Duration milliseconds,
                                        TimeOutCallback callback, bool cyclic) {
  auto jid = idManager_.allocateNewID();
  if (jid != IDManager::INVALID_ID) {
    auto jobDoneCallback = [callback, this](JobID jid, bool isCyclic) {
      if (!isCyclic) {
        idManager_.reclaimUsedID(jid);
      }
      if (callback) {
        callback();
      }
    };

    if (!pImpl_->start(jid, milliseconds, jobDoneCallback, cyclic)) {
      idManager_.reclaimUsedID(jid);
    }
  }
  return jid;
}

void TimerManager::stop(TimerManager::JobID jid) {
  if (isValid(jid)) {
    idManager_.reclaimUsedID(jid);
    pImpl_->stop(jid);
  }
}

void TimerManager::stop() { pImpl_->stop(); }

bool TimerManager::isRunning(TimerManager::JobID jid) {
  return isValid(jid) && pImpl_->isRunning(jid);
}

void TimerManager::setCyclic(TimerManager::JobID jid, bool cyclic) {
  if (isValid(jid)) {
    pImpl_->setCyclic(jid, cyclic);
  }
}

bool TimerManager::isValid(TimerManager::JobID jid) {
  return IDManager::isValidID(jid);
}

} // namespace messaging
} // namespace maf
