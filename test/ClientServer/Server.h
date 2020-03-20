#pragma once

#include "WeatherContract.h"
#include <chrono>
#include <maf/messaging/AsyncCallbackExecutor.h>
#include <maf/messaging/ExtensibleComponent.h>
#include <maf/messaging/Timer.h>
#include <maf/messaging/client-server/Stub.h>
#include <maf/utils/TimeMeasurement.h>

using namespace maf::messaging;
using namespace std::chrono;

static const std::string WEATHER_DATA[] = {"Suny", "Windy", "Rainy"};

std::vector<std::string> createBigExtraInfomation(size_t dataSize) {
  std::vector<std::string> v = {"nguyen", "van", "con"};
  v.reserve(dataSize);
  for (size_t i = 0; i < dataSize; ++i) {
    v.push_back("nguyen ");
  }
  return v;
}

std::string createBigString(size_t size, const std::string &tobeCloned) {
  std::string s;
  s.reserve(size * tobeCloned.size());
  for (size_t i = 0; i < size; ++i) {
    s += tobeCloned;
  }
  return s;
}
static std::vector<std::string> extraInfomation = createBigExtraInfomation(1);
static std::string SStatus = createBigString(
    1,
    "Hello world"); //"{\"data_id\":\"a4cb90f84d2448009ecc48f6b7ed0c7e\",\"dlp_info\":{},\"file_info\":{\"display_name\":\"componentsplugin4.dll\",\"file_size\":100864,\"file_type\":\"application/x-dosexec\",\"file_type_description\":\"Dynamic
                    // Link
                    // Library\",\"md5\":\"c713c6f0ea073c1822933aa5be4f1794\",\"sha1\":\"70a4cdf21b39bae2721a0fa84716eca6229ff946\",\"sha256\":\"4644c6f5556414d92eecd3792358fa2ca80a0988469a82f33e928be237420881\",\"upload_timestamp\":\"2019-07-19T03:01:13.471Z\"},\"process_info\":{\"blocked_reason\":\"\",\"file_type_skipped_scan\":false,\"post_processing\":{\"actions_failed\":\"\",\"actions_ran\":\"\",\"converted_destination\":\"\",\"converted_to\":\"\",\"copy_move_destination\":\"\"},\"processing_time\":76,\"profile\":\"File
                    // process\",\"progress_percentage\":100,\"queue_time\":7,\"output\":\"Allowed\",\"user_agent\":\"MetaAccess\"},\"scan_results\":{\"data_id\":\"a4cb90f84d2448009ecc48f6b7ed0c7e\",\"progress_percentage\":100,\"scan_all_output_a\":\"No
                    // Threat
                    // Detected\",\"scan_all_output_i\":0,\"scan_details\":{\"Ahnlab\":{\"def_time\":\"2019-07-19T00:00:00.000Z\",\"eng_id\":\"ahnlab_1_windows\",\"location\":\"local\",\"scan_output_i\":0,\"scan_time\":21,\"threat_found\":\"\",\"wait_time\":7},\"Avira\":{\"def_time\":\"2019-07-17T00:00:00.000Z\",\"eng_id\":\"avira_1_windows\",\"location\":\"local\",\"scan_output_i\":0,\"scan_time\":7,\"threat_found\":\"\",\"wait_time\":7},\"ClamAV\":{\"def_time\":\"2019-07-18T08:12:00.000Z\",\"eng_id\":\"clamav_1_windows\",\"location\":\"local\",\"scan_output_i\":0,\"scan_time\":23,\"threat_found\":\"\",\"wait_time\":11},\"ESET\":{\"def_time\":\"2019-07-18T00:00:00.000Z\",\"eng_id\":\"eset_1_windows\",\"location\":\"local\",\"scan_output_i\":0,\"scan_time\":17,\"threat_found\":\"\",\"wait_time\":11}},\"start_time\":\"2019-07-19T03:01:13.478Z\",\"total_avs\":4,\"total_time\":69},\"vulnerability_info\":{\"verdict\":0},\"yara_info\":{}}";

using namespace std::chrono;

inline auto createBigStringList(const std::string &item = "hello world",
                                size_t size = 1000) {
  today_weather_request::StringList list;
  for (size_t i = 0; i < size; ++i) {
    list.push_back(item);
  }
  return list;
}

template <class Stub>
class ServerComponent : public maf::messaging::ExtensibleComponent {
  template <class Input>
  using RequestPtr = typename Stub::template RequestPtr<Input>;

public:
  ServerComponent(std::shared_ptr<Stub> stub) : stub_{std::move(stub)} {
    stub_->setExecutor(asyncExecutor(component()));
    MAF_LOGGER_DEBUG("Service id is: ", stub_->serviceID());
    stub_->template registerRequestHandler<clear_all_status_request>(
        [this](const auto &request) {
          MAF_LOGGER_DEBUG("Received clear status request....");
          this->resetAllStatuses();
          //            request->error("Some error occurred while clearing
          //            statuses!");
          request->respond();
        });

    auto success =
        stub_->template registerRequestHandler<update_status_request>(
            [this](const auto &request) {
              MAF_LOGGER_DEBUG("Received status update request....");
              this->setStatus();
              request->respond();

              for (int i = 0; i < 10; ++i) {
                maf::util::TimeMeasurement tm{[](auto elapsedMcs) {
                  MAF_LOGGER_DEBUG(
                      "Time to set compliance 5 status = ",
                      duration_cast<milliseconds>(elapsedMcs).count());
                }};

                auto status =
                    stub_->template getStatus<compliance5_property::status>();
                MAF_LOGGER_DEBUG("The value Server set to client: ",
                                 status->dump());
              }
            });
    if (!success) {
      MAF_LOGGER_DEBUG("Failed to register request handler for message",
                       update_status_request::operationID());
    }

    stub_->template registerRequestHandler<broad_cast_signal_request>(
        [this](const RequestPtr<broad_cast_signal_request> &request) {
          request->respond();
          MAF_LOGGER_DEBUG("Received broad cast signal request....");
          this->broadcastSignal();
        });

    stub_->template registerRequestHandler<boot_time_request>(
        [this](const auto &request) {
          using namespace std::chrono;
          MAF_LOGGER_DEBUG("Received boot_time status get request....");
          request->template respond<boot_time_request::output>(
              duration_cast<seconds>(system_clock::now() - this->_bootTime)
                  .count());
        });

    stub_->template registerRequestHandler<shutdown_request>(
        [this](const auto &request) {
          static auto retried = 0;

          MAF_LOGGER_DEBUG("Recevied shutdown request from client!");
          if (++retried == 10) {
            MAF_LOGGER_DEBUG(
                "Shutdown server due to many requests from client!");
            request->respond();
            stop();
          } else {
            request->error("Client is not allowed to shutdown server",
                           CSErrorCode::RequestRejected);
          }
        });

    stub_->template registerRequestHandler<today_weather_request::input>(
        [](const RequestPtr<today_weather_request::input> &request) {
          if (auto input = request->getInput()) {
            MAF_LOGGER_DEBUG("Got today_weather request from client: ",
                             input->dump());
            if (input->get_pid() == -1) {
              request->error("Invalid pid", CSErrorCode::InvalidParam);
            } else if (input->get_pid() % 2 == 0) {
              request->error("Doesn't support request with even pid value",
                             CSErrorCode::InvalidParam);
            } else {
              auto output = today_weather_request::make_output();
              output->set_list_of_places(createBigStringList());
              request->respond(std::move(output));
            }
          } else {
            request->error("Request contains no input",
                           CSErrorCode::InvalidParam);
          }
        });

    stub_->startServing();
  }

  void onExit() override {
    MAF_LOGGER_DEBUG("Server Component is shutting down!");
    stub_->stopServing();
  }

  void resetAllStatuses() {
    stub_->setStatus(compliance5_property::make_status());
    stub_->setStatus(compliance4_property::make_status());
    stub_->setStatus(compliance3_property::make_status());
    stub_->setStatus(compliance2_property::make_status());
    stub_->setStatus(compliance1_property::make_status());
    MAF_LOGGER_DEBUG("Clear status done!");
  }

  void setStatus() {
    stub_->template setStatus<compliance5_property::status>(false, true, 0,
                                                            SStatus);
    stub_->template setStatus<compliance4_property::status>(false, true, 0,
                                                            "hello world");
    stub_->template setStatus<compliance3_property::status>(false, true, 0,
                                                            "hello world");
    stub_->template setStatus<compliance2_property::status>(false, true, 0,
                                                            "hello world");
    stub_->template setStatus<compliance1_property::status>(false, true, 0,
                                                            "hello world");
  }

  void broadcastSignal() {
    stub_->template broadcastSignal<server_request_signal>();
    stub_->template broadcastSignal<client_info_request_signal::attributes>(
        "nocpes.nocpes");
  }

private:
  system_clock::time_point _bootTime = system_clock::now();
  std::shared_ptr<Stub> stub_;
};
