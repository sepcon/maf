#ifndef SERVICESTATUSSIGNAL_H
#define SERVICESTATUSSIGNAL_H

#include "BasicProxy.h"
#include <condition_variable>
#include <mutex>

namespace maf {
namespace messaging {

class ServiceStatusSignal : public ServiceStatusObserverIF {
public:
  bool waitTill(const Availability expectedStatus) {
    std::unique_lock lock(m_);
    statusChanged_.wait(lock, [this, expectedStatus] {
      return expectedStatus == currentStatus_ || stopped_;
    });
    if (!stopped_) {
      return true;
    }
    return false;
  }

  bool waitTill(const Availability expectedStatus, long long ms) {
    return waitTill(expectedStatus, std::chrono::milliseconds{ms});
  }

  template <typename Rep, typename Period>
  bool waitTill(const Availability expectedStatus,
                const std::chrono::duration<Rep, Period> interval) {
    std::unique_lock lock(m_);
    if (statusChanged_.wait_for(lock, interval, [this, expectedStatus] {
          return expectedStatus == currentStatus_ || stopped_;
        })) {
      if (!stopped_) {
        return true;
      }
    }
    return false;
  }

  void stop() {
    std::lock_guard lock(m_);
    stopped_ = true;
    statusChanged_.notify_all();
  }

  void onServiceStatusChanged(const ServiceID & /*sid*/,
                              Availability /*oldStatus*/,
                              Availability newStatus) noexcept override {
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

template <class ProxyType>
std::shared_ptr<ServiceStatusSignal>
serviceStatusSignal(const std::shared_ptr<ProxyType> &proxy = {}) {
  auto waiter = serviceStatusSignal();
  if (proxy) {
    proxy->registerServiceStatusObserver(waiter);
  }
  return waiter;
}

std::shared_ptr<ServiceStatusSignal> serviceStatusSignal() {
  return std::shared_ptr<ServiceStatusSignal>{new ServiceStatusSignal{}};
}

} // namespace messaging
} // namespace maf

#endif // SERVICESTATUSSIGNAL_H
