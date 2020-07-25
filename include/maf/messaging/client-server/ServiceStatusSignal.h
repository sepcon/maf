#pragma once

#include <condition_variable>
#include <mutex>

#include "BasicProxy.h"

namespace maf {
namespace messaging {

class ServiceStatusSignal : public ServiceStatusObserverIF {
 public:
  class WaitStatus {
   public:
    enum Status : char { Ready, TimeOut, Interrupted };
    WaitStatus(Status s) : _{s} {}
    bool isReady() const { return _ == Ready; }
    bool isInterrupted() const { return _ == Interrupted; }
    bool isTimeout() const { return _ == TimeOut; }

   private:
    Status _;
  };

  WaitStatus waitIfNot(const Availability expectedStatus) {
    std::unique_lock lock(m_);
    statusChanged_.wait(lock, [this, expectedStatus] {
      return expectedStatus == currentStatus_ || stopped_;
    });
    if (!stopped_) {
      return WaitStatus(WaitStatus::Ready);
    }
    return WaitStatus(WaitStatus::Interrupted);
  }

  WaitStatus waitIfNot(const Availability expectedStatus, long long ms) {
    return waitIfNot(expectedStatus, std::chrono::milliseconds{ms});
  }

  template <typename Rep, typename Period>
  WaitStatus waitIfNot(const Availability expectedStatus,
                       const std::chrono::duration<Rep, Period> interval) {
    std::unique_lock lock(m_);
    if (statusChanged_.wait_for(lock, interval, [this, expectedStatus] {
          return expectedStatus == currentStatus_ || stopped_;
        })) {
      if (!stopped_) {
        return WaitStatus(WaitStatus::Ready);
      } else {
        return WaitStatus(WaitStatus::Interrupted);
      }
    } else {
      return WaitStatus(WaitStatus::TimeOut);
    }
  }

  void stop() {
    std::lock_guard lock(m_);
    stopped_ = true;
    statusChanged_.notify_all();
  }

  void onServiceStatusChanged(const ServiceID & /*sid*/,
                              Availability /*oldStatus*/,
                              Availability newStatus) override {
    std::lock_guard lock(m_);
    currentStatus_ = newStatus;
    statusChanged_.notify_all();
  }

  Availability currentStatus() const {
    std::lock_guard lock(m_);
    return currentStatus_;
  }

 private:
  mutable std::mutex m_;
  std::condition_variable statusChanged_;
  Availability currentStatus_ = Availability::Unavailable;
  bool stopped_ = false;
};

std::shared_ptr<ServiceStatusSignal> serviceStatusSignal() {
  return std::shared_ptr<ServiceStatusSignal>{new ServiceStatusSignal{}};
}

template <class PTrait>
std::shared_ptr<ServiceStatusSignal> serviceStatusSignal(
    const std::shared_ptr<BasicProxy<PTrait>> &proxy = {}) {
  auto waiter = serviceStatusSignal();
  if (proxy) {
    proxy->registerServiceStatusObserver(waiter);
  }
  return waiter;
}

}  // namespace messaging
}  // namespace maf
