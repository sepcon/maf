#pragma once

#include <maf/Messaging.h>

#include <iostream>
#include <thread>

#include "client-server-contract.h"
#include "maf/messaging/client-server/BasicProxy.h"
#include "maf/utils/TimeMeasurement.h"

using namespace std::chrono_literals;
using namespace maf;
using namespace maf::messaging;

struct EndOfRequestChainMsg {};

template <class Proxy>
class ClientProcessor : public ProcessorEx {
  using ProxyPtr = std::shared_ptr<Proxy>;
  template <class T>
  using Response = typename Proxy::template Response<T>;

 public:
  ClientProcessor(ProxyPtr proxy) : proxy_{std::move(proxy)} {
    proxy_->setExecutor(instance()->getExecutor());

    statusObserver_ = proxy_->onServiceStatusChanged(
        [this](Availability, Availability newStatus) {
          if (newStatus == Availability::Available) {
            proxy_->template sendRequest<unhandled_request>(nullptr, 10s);
            proxy_->template sendRequestAsync<implicitly_response_request>(
                [](auto response) {
                  assert(response.isError());
                  auto err = response.getError();
                  MAF_LOGGER_DEBUG("Got implicit response from server ",
                                   err->dump());
                });

            MAF_LOGGER_DEBUG(
                "Server already clear all status, then start jobs...");
            registerStatuses();

            proxy_->template sendRequestAsync<today_weather_request::output>(
                today_weather_request::make_input("Client is client", 100),
                [](Response<today_weather_request::output> response) {
                  if (response.isOutput()) {
                    MAF_LOGGER_DEBUG("Received output from server ",
                                     response.getOutput()->get_your_command());
                  } else {
                    MAF_LOGGER_DEBUG("Error for request of today_weather: ",
                                     response.getError()->description());
                  }
                });

            proxy_->template sendRequestAsync<update_status_request>();

          } else {
            MAF_LOGGER_DEBUG(
                "Service is off for sometime, please wait for him "
                "to be available again!");
          }
        });

    // Try send first request when service is not available,
    // with asumption that server started later than client
    proxy_->template sendRequest<boot_time_request::output>(nullptr, 10s);
  }

  ~ClientProcessor() {
    for (auto &regid : regids_) {
      if (auto callstatus = proxy_->unregister(regid);
          callstatus != ActionCallStatus::Success) {
        MAF_LOGGER_INFO("Failed to unregister status: ", regid.opID,
                        " with error: ", callstatus);
      } else {
        MAF_LOGGER_DEBUG("Successfully unregistered status ", regid.opID);
      }
    }

    proxy_->unregisterServiceStatusObserver(statusObserver_);
  }

  template <typename Category>
  void sendSyncRequest() {
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

  template <class Status>
  void registerStatus() {
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
      MAF_LOGGER_INFO("Failed to register property ", Status::operationID(),
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
    if (auto response =
            proxy_->template sendRequest<boot_time_request::output>();
        response.isOutput()) {
      if (auto output = response.getOutput()) {
        MAF_LOGGER_DEBUG("server life is ",
                         response.getOutput()->get_seconds());

        if (response.getOutput()->get_seconds() > 10) {
          auto shutdownRequestResponse =
              proxy_->template sendRequest<shutdown_request>();
          if (shutdownRequestResponse.isError()) {
            MAF_LOGGER_DEBUG("Failed to shutdown server: ",
                             shutdownRequestResponse.getError()->description());
          } else {
            MAF_LOGGER_DEBUG("Server already shutdown!");
          }
        }
      } else if (auto error = response.getError()) {
        MAF_LOGGER_DEBUG("Got error from server: ", error->dump());
      } else {
        MAF_LOGGER_INFO("Server might respond nothing!");
      }
    }
    this_processor::post<EndOfRequestChainMsg>();
  }
  template <class Status>
  void getStatus() {
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
