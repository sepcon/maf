#pragma once

#include <thread>

#include "WeatherContract.h"
#include "maf/messaging/AsyncCallbackExecutor.h"
#include "maf/messaging/ExtensibleComponent.h"
#include "maf/messaging/Timer.h"
#include "maf/messaging/client-server/AsyncServiceStatusObserver.h"
#include "maf/messaging/client-server/Proxy.h"
#include "maf/utils/TimeMeasurement.h"

using namespace std::chrono_literals;
using namespace maf;
using namespace maf::messaging;

struct EndOfRequestChainMsg {};

template <class Proxy> class ClientComponent : public ExtensibleComponent {
  using ProxyPtr = std::shared_ptr<Proxy>;
  template <class T>
  using ResponsePtr = typename Proxy::template ResponsePtr<T>;

public:
  ClientComponent(ProxyPtr proxy) : proxy_{std::move(proxy)} {
    proxy_->setExecutor(asyncExecutor(component()));
    proxy_->registerServiceStatusObserver(
        statusObserver_ = asyncServiceStatusObserver(component()));

    // Try send first request when service is not available,
    // with asumption that server started later than client
    proxy_->template sendRequest<boot_time_request::output>(nullptr, 10s);

    onMessage<ServiceStatusMsg>([this](ServiceStatusMsg msg) {
      if (msg.newStatus == Availability::Available) {
        auto response =
            proxy_->template sendRequest<unhandled_request>(nullptr, 10s);
        MAF_LOGGER_DEBUG("Server already clear all status, then start jobs...");
        registerStatuses();

        proxy_->template sendRequestAsync<today_weather_request::output>(
            today_weather_request::make_input("Client is client", 100),
            [](const ResponsePtr<today_weather_request::output> &response) {
              if (*response) {
                MAF_LOGGER_DEBUG("Received output from server ",
                                 response->getOutput()->get_your_command());
              } else {
                MAF_LOGGER_DEBUG("Error for request of today_weather: ",
                                 response->getError()->description());
              }
            });

        proxy_->template sendRequestAsync<update_status_request>();

      } else {
        MAF_LOGGER_DEBUG("Service is off for sometime, please wait for him "
                         "to be available again!");
      }
    });
  }

  ~ClientComponent() {
    for (auto &regid : regids_) {
      if (auto callstatus = proxy_->unregisterBroadcast(regid);
          callstatus != ActionCallStatus::Success) {
        MAF_LOGGER_ERROR("Failed to unregister status: ", regid.opID,
                         " with error: ", callstatus);
      } else {
        MAF_LOGGER_DEBUG("Successfully unregistered status ", regid.opID);
      }
    }

    proxy_->unregisterServiceStatusObserver(statusObserver_);
  }

  template <typename Category> void sendSyncRequest() {
    long long total = 0;
    const int totalRequests = 5000;
    {
      util::TimeMeasurement tm{
          [&total](util::TimeMeasurement::MicroSeconds elapsed) {
            total += elapsed.count();
          }};
      for (int i = 0; i < totalRequests; ++i) {
        proxy_->template sendRequest<struct Category::output>(
            Category::make_input(), 500);
      }
    }
    auto avarageTimePerRequest = static_cast<double>(total) / totalRequests;
    MAF_LOGGER_DEBUG("Avarage time to send a request is: ",
                     avarageTimePerRequest, " microseconds");
  }

  void registerStatuses() {
    registerStatus<compliance5_property::status>();
    registerStatus<compliance4_property::status>();
    registerStatus<compliance3_property::status>();
    registerStatus<compliance2_property::status>();
    registerStatus<compliance1_property::status>();
  }

  template <class Status> void registerStatus() {
    auto dumpCallback = [this](const auto &status) {
      MAF_LOGGER_DEBUG("Got status update from server[", status->operationID(),
                       "]: ", status->get_updated_count());
      if (status->operationID() == compliance1_property::ID) {
        this->getStatuses();
      }
    };

    ActionCallStatus callStatus;
    RegID regid =
        proxy_->template registerStatus<Status>(dumpCallback, &callStatus);
    if (regid.valid()) {
      regids_.push_back(std::move(regid));
    } else {
      MAF_LOGGER_ERROR("Failed to register property ", Status::operationID(),
                       " with call status = ", callStatus);
    }
  }

  void getStatuses() {
    getStatus<compliance_property::status>();
    getStatus<compliance5_property::status>();
    getStatus<compliance1_property::status>();
    registerSignal();
  }

  void registerSignal() {
    proxy_->template registerSignal<server_request_signal>([]() {
      MAF_LOGGER_DEBUG("Received ", server_request_signal::ID, " from server");
    });

    proxy_->template registerSignal<client_info_request_signal::attributes>(
        [this](const auto &signal) {
          MAF_LOGGER_DEBUG("Received signal ", client_info_request_signal::ID,
                           "from server: ", signal->dump());

          tryStopServer();
        });

    proxy_->template sendRequestAsync<broad_cast_signal_request>();
  }
  void tryStopServer() {
    if (auto lastBootTime =
            proxy_->template sendRequest<boot_time_request::output>();
        lastBootTime && lastBootTime->isOutput()) {
      if (auto output = lastBootTime->getOutput()) {
        MAF_LOGGER_DEBUG("server life is ",
                         lastBootTime->getOutput()->get_seconds());
        if (lastBootTime->getOutput()->get_seconds() > 10) {
          if (auto response =
                  proxy_->template sendRequest<shutdown_request>()) {
            if (response->isError()) {
              MAF_LOGGER_DEBUG("Failed to shutdown server: ",
                               response->getError()->description());
            } else {
              MAF_LOGGER_DEBUG("Server already shutdown!");
            }
          }
        }
      } else if(auto error = lastBootTime->getError()) {
          MAF_LOGGER_DEBUG("Got error from server: ", error->dump());
      } else {
          MAF_LOGGER_ERROR("Don't know why response contains nothing!");
      }
    }
    RunningComponent::post<EndOfRequestChainMsg>();
  }
  template <class Status> void getStatus() {
    for (int i = 0; i < 10; ++i) {
      auto compliance5 = proxy_->template getStatus<Status>();
      if (compliance5) {
        MAF_LOGGER_DEBUG(
            "Got update from server, status id = ", Status::operationID(),
            compliance5->get_updated_count());
      } else {
        MAF_LOGGER_DEBUG("Got empty status from server for status id ",
                         Status::operationID());
      }
    }

    auto callstatus =
        proxy_->template getStatus<Status>([](const auto &status) {
          if (status) {
            MAF_LOGGER_INFO("Get status async of `", Status::operationID(),
                            "`: ", status->dump());
          }
        });
    if (callstatus != ActionCallStatus::Success) {
      MAF_LOGGER_INFO("Failed to get status async of `", Status::operationID(),
                      "`: with callstatus = ", callstatus);
    }
  }

private:
  std::shared_ptr<ServiceStatusObserverIF> statusObserver_;
  std::shared_ptr<Proxy> proxy_;
  std::vector<RegID> regids_;
  ServiceID sid_;
};
