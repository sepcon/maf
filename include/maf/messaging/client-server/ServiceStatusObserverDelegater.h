#pragma once

#include <maf/messaging/Exceptions.h>
#include <maf/messaging/ExecutorIF.h>

#include "ServiceStatusObserverIF.h"

namespace maf {
namespace messaging {

class ServiceStatusObserverDelegater : public ServiceStatusObserverIF {
 public:
  using DelegateCallback = std::function<void(Availability, Availability)>;

  ServiceStatusObserverDelegater(ExecutorPtr executor,
                                 DelegateCallback callback)
      : callback_(std::move(callback)), executor_(std::move(executor)) {}

  void onServiceStatusChanged(const ServiceID &, Availability oldStatus,
                              Availability newStatus) override {
    if (!executor_->execute(std::bind(callback_, oldStatus, newStatus))) {
      throw UnavailableException{};
    }
  }

 private:
  DelegateCallback callback_;
  ExecutorPtr executor_;
};

}  // namespace messaging
}  // namespace maf
