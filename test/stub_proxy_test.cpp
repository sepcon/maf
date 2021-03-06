#include <maf/ITCProxy.h>
#include <maf/ITCStub.h>
#include <maf/LocalIPCProxy.h>
#include <maf/LocalIPCStub.h>
#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/ServiceStatusSignal.h>
#include <maf/threading/AtomicObject.h>
#include <maf/utils/DirectExecutor.h>
#include <maf/utils/TimeMeasurement.h>

#include <algorithm>
#include <future>
#include <iostream>
#include <map>
#include <set>

#include "test.h"

// clang-format off
#include <maf/messaging/client-server/CSContractDefinesBegin.mc.h>
REQUEST(string)
    INPUT((std::string, string_input))
    OUTPUT((std::string, string_output))
ENDREQUEST(string_request)

REQUEST(convert_ints_2_strings)
    INPUT((std::vector<int>, ints))
    OUTPUT((std::vector<std::string>, sints))
ENDREQUEST(convert_ints_2_strings)

REQUEST(map_string_vector_2_string)
using the_map = std::map<std::string, std::vector<int>>;
    INPUT((the_map, the_map))
    OUTPUT((std::string, map_as_string))
ENDREQUEST(map_string_vector)

VOID_REQUEST(to_be_aborted)

VOID_REQUEST(no_response)

SIGNAL(server_notify)
    using DetailsMap = std::map<std::string, std::string>;
    ATTRIBUTES
    (
        (std::string, notification),
        (DetailsMap, details)
    )
ENDSIGNAL(server_notify)

PROPERTY(some_string)
    STATUS((std::string, its_status))
ENDPROPERTY()

PROPERTY(varied_string)
	STATUS((std::string, its_status))
ENDPROPERTY()

// clang-format on

#include <maf/messaging/client-server/CSContractDefinesEnd.mc.h>

using namespace maf::messaging;
using namespace maf::util;
namespace localipc = maf::localipc;
namespace itc = maf::itc;
using namespace std::chrono_literals;

template <class PTrait>
class Tester {
  using Stub = maf::messaging::BasicStub<PTrait>;
  using Proxy = maf::messaging::BasicProxy<PTrait>;
  template <class Input>
  using Request = typename Stub::template Request<Input>;
  template <class Output>
  using Response = typename Proxy::template Response<Output>;

 public:
  Tester(std::shared_ptr<Stub> stub, std::shared_ptr<Proxy> proxy)
      : stub_{std::move(stub)}, proxy_{std::move(proxy)} {}

  void test() {
    maf::util::TimeMeasurement tm([](auto elapsed) {
      maf::test::log_rec() << "Total time for test = " << elapsed.count() / 1000
                           << "ms";
    });

    std::shared_ptr<Request<no_response_request>> noResponseRequestKeeper;

    auto stub = stub_->with(maf::util::directExecutor());
    auto proxy = proxy_->with(maf::util::directExecutor());
    Availability serviceStatus;
    stub->template registerRequestHandler<string_request::input>(
        [](Request<string_request::input> request) {
          auto input = request.getInput();
          // 0. Confirm input is not null
          assert(input != nullptr);
          if (input->get_string_input() != "ignore_me") {
            // 1. Respond same value as input string
            request.template respond<string_request::output>(
                input->get_string_input());
          }
        });

    stub->template registerRequestHandler<
        convert_ints_2_strings_request::input>(
        [](Request<convert_ints_2_strings_request::input> request) {
          // 0. Confirm input is not null
          auto input = request.getInput();
          assert(input);
          // 1. create output object
          auto output = convert_ints_2_strings_request::make_output();
          // 2. Get reference to input data vector of integers
          const auto& ints = input->get_ints();
          // 3. transform integers to corresponding strings
          std::transform(std::begin(ints), std::end(ints),
                         std::back_inserter(output->get_sints()),
                         [](int val) { return std::to_string(val); });

          // 4. Respond output to request
          request.respond(std::move(output));
        });

    stub->template registerRequestHandler<
        map_string_vector_2_string_request::input>(
        [](Request<map_string_vector_2_string_request::input> request) {
          auto input = request.getInput();
          // 0. Confirm input is not null
          assert(input);
          // 1. create output object
          auto output = map_string_vector_2_string_request::make_output();
          // 3. dump the map to json like string
          output->set_map_as_string(input->dump());
          // 4. Respond output to request
          request.respond(std::move(output));
        });

    stub->template registerRequestHandler<no_response_request>(
        [&noResponseRequestKeeper, stub](auto request) {
          // Keep request to make it never be able to respond
          noResponseRequestKeeper =
              std::make_shared<Request<no_response_request>>(
                  std::move(request));
          stub->stopServing();
        });

    stub_->startServing();

    std::promise<void> serviceStatusSource;
    auto ftServiceStatusChangedSignal = serviceStatusSource.get_future();
    proxy->onServiceStatusChanged(
        [&serviceStatus, &serviceStatusSource](auto, Availability newStatus) {
          serviceStatus = newStatus;
          serviceStatusSource.set_value();
        });

    serviceStatusSignal(proxy)->waitIfNot(Availability::Available);

    TEST_CASE_B(service_status) {
      EXPECT(ftServiceStatusChangedSignal.wait_for(10ms) ==
             std::future_status::ready);
      EXPECT(serviceStatus == Availability::Available);
    }
    TEST_CASE_E(service_status)

    TEST_CASE_B(request_response_string) {
      // 1. Send string_request and expect same response
      auto inputString = std::string{"Hello world"};
      auto response = proxy->template sendRequest<string_request::output>(
          string_request::make_input(inputString));
      // 2. Confirm having output
      EXPECT(response.isOutput());
      // 3. Confirm response's output == inputString
      EXPECT(response.getOutput()->get_string_output() == inputString);
    }
    TEST_CASE_E(String_request_response)

    TEST_CASE_B(request_response_vector) {
      auto ints = std::vector<int>{1, 2, 3};
      auto expectedStrings = std::vector<std::string>{};
      std::transform(std::begin(ints), std::end(ints),
                     std::back_inserter(expectedStrings),
                     [](int v) { return std::to_string(v); });

      // 1. Send integers to server
      auto response =
          proxy->template sendRequest<convert_ints_2_strings_request::output>(
              convert_ints_2_strings_request::make_input(std::move(ints)));
      // 2. Confirm having output
      EXPECT(response.isOutput());

      // 3. Confirm response's output == expectedStrings
      EXPECT(response.getOutput()->get_sints() == expectedStrings);
    }
    TEST_CASE_E(request_response_vector)

    TEST_CASE_B(request_response_map_string_2_vector) {
      map_string_vector_2_string_request::the_map themap = {
          {"one two three", {1, 2, 3}}, {"four five six", {4, 5, 6}}};

      auto expectedResponseString = std::string{};
      maf::srz::DumpHelper<map_string_vector_2_string_request::the_map>::dump(
          themap, 2, expectedResponseString);

      auto input =
          map_string_vector_2_string_request::make_input(std::move(themap));
      // 1. Send themap to server
      auto response = proxy->template sendRequest<
          map_string_vector_2_string_request::output>(input);
      // 2. Confirm having output
      EXPECT(response.isOutput());

      // 3. Confirm response's output == expectedResponseString
      EXPECT(response.getOutput()->get_map_as_string() == input->dump());
    }
    TEST_CASE_E(request_response_map_string_2_vector)

    TEST_CASE_B(request_but_failed_to_response) {
      // 1. Send string_request and expect same response
      auto inputString = std::string{"ignore_me"};
      auto response = proxy->template sendRequest<string_request::output>(
          string_request::make_input(inputString));
      // 2. Confirm having output
      EXPECT(response.isError());
      // 3. Confirm response's output == inputString
      EXPECT(response.getError()->code() == CSErrorCode::ResponseIgnored);
    }
    TEST_CASE_E(request_but_failed_to_response)

    TEST_CASE_B(abort_request) {
      bool aborted = false;
      auto requestComeEventSource = std::make_shared<std::promise<void>>();
      auto requestComeEvent = requestComeEventSource->get_future();
      std::shared_ptr<Request<to_be_aborted_request>> requestHolder;
      stub->template registerRequestHandler<to_be_aborted_request>(
          [&aborted, &requestComeEventSource,
           &requestHolder](Request<to_be_aborted_request> request) {
            requestComeEventSource->set_value();
            request.onAborted([&aborted] { aborted = true; }, directExecutor());

            // Keep the request to make it not auto respond error in its
            // destructor
            requestHolder = std::make_shared<Request<to_be_aborted_request>>(
                std::move(request));
          });

      auto gotResponse = false;
      auto regid = proxy->template sendRequestAsync<to_be_aborted_request>(
          [&gotResponse](auto) { gotResponse = true; });

      EXPECT(requestComeEvent.wait_for(10ms) == std::future_status::ready);

      EXPECT(aborted == false);

      proxy->abortRequest(regid);
      std::this_thread::sleep_for(1ms);

      EXPECT(aborted == true);

      // After aborted expect that the Request<to_be_aborted_request> cant
      // respond anymore
      EXPECT(requestHolder->respond() == ActionCallStatus::InvalidCall);

      //        std::this_thread::sleep_for(1ms);

      // Due to request is aborted, then gotResponse will never meet
      EXPECT(gotResponse == false);
    }
    TEST_CASE_E(abort_request)

    TEST_CASE_B(broad_cast_status_signal) {
      // 1. Send string_request and expect same response
      auto inputString = std::string{"ignore_me"};
      server_notify_signal::DetailsMap details = {{"key", "value"},
                                                  {"key1", "value1"}};
      std::promise<server_notify_signal::attributes> receivedAttribute;
      auto receivedAttributeFuture = receivedAttribute.get_future();
      auto sentAttribute =
          server_notify_signal::make_attributes(inputString, details);

      auto regid =
          proxy->template registerSignal<server_notify_signal::attributes>(
              [&receivedAttribute](server_notify_signal::attributes_cptr attr) {
                receivedAttribute.set_value(std::move(*attr));
              });

      // wait for signal register comes to server
      std::this_thread::sleep_for(1ms);

      stub->broadcastSignal(sentAttribute);

      EXPECT(receivedAttributeFuture.wait_for(1000ms) ==
             std::future_status::ready);
      EXPECT(receivedAttributeFuture.get() == *sentAttribute);

      proxy->unregister(regid);

      auto sentStatus = some_string_property::make_status("this is status");

      using PropertyReg =
          std::tuple<RegID, std::future<some_string_property::status>>;
      std::vector<PropertyReg> propertyRegs;
      constexpr size_t MAX_REGISTERS = 10;

      for (size_t i = 0; i < MAX_REGISTERS; ++i) {
        // The property's status will be cached in client side, then it must be
        // shared between all registers
        auto someStringStatusProms =
            std::make_shared<std::promise<some_string_property::status>>();
        auto someStringStatusFuture = someStringStatusProms->get_future();
        auto regID =
            proxy->template registerStatus<some_string_property::status>(
                [proms = std::move(someStringStatusProms)](
                    some_string_property::status_cptr status) mutable {
                  proms->set_value(std::move(*status));
                });

        propertyRegs.emplace_back(std::move(regID),
                                  std::move(someStringStatusFuture));
      }

      for (size_t i = 0; i < MAX_REGISTERS - 1; ++i) {
        auto& regIDi = std::get<0>(propertyRegs[i]);
        auto& regIDi1 = std::get<0>(propertyRegs[i + 1]);
        EXPECT(regIDi != regIDi1);
      }

      // wait for signal register comes to server
      std::this_thread::sleep_for(1ms);

      stub_->setStatus(sentStatus);
      stub_->template setStatus<some_string_property::status>("hello");

      for (auto& [regID, propFuture] : propertyRegs) {
        do {
          EXPECT(propFuture.wait_for(100ms) == std::future_status::ready);
          EXPECT(propFuture.get() == *sentStatus);
        } while (false);
        proxy->unregister(regID);
      }

      sentStatus->set_its_status("sdfdsfdsfdsfds");
      stub_->setStatus(sentStatus);

      auto getBackedStatus =
          stub_->template getStatus<some_string_property::status>();
      EXPECT(getBackedStatus && *getBackedStatus == *sentStatus);

      auto gotStatus =
          proxy->template getStatus<some_string_property::status>();
      EXPECT(gotStatus);
      maf::test::log_rec() << gotStatus->dump();
      EXPECT(*gotStatus == *sentStatus);

      std::set<std::string> statusesToUpdate = {"1", "2", "3", "4", "5"};
      maf::threading::AtomicObject<std::set<std::string>> updatedStatuses;
      auto getAllSignalSource = std::make_shared<std::promise<void>>();
      auto getAllSignal = getAllSignalSource->get_future();

      proxy->template registerStatus<varied_string_property::status>(
          [&updatedStatuses, getAllSignalSource,
           totalUpdate = statusesToUpdate.size()](
              varied_string_property::status_ptr status) {
            updatedStatuses->insert(status->get_its_status());
            if (updatedStatuses->size() == totalUpdate) {
              getAllSignalSource->set_value();
            }
          });

      for (auto& s : statusesToUpdate) {
        stub->template setStatus<varied_string_property::status>(s);
        std::this_thread::sleep_for(1ms);
      }

      EXPECT(getAllSignal.wait_for(10ms) == std::future_status::ready);
      EXPECT(statusesToUpdate == updatedStatuses.lockee());
    }
    TEST_CASE_E(broad_cast_status_signal)

    auto callstatus = ActionCallStatus{};
    auto response =
        proxy->template sendRequest<no_response_request>(&callstatus);

    TEST_CASE_B(stopable_sync_request) {
      EXPECT(callstatus == ActionCallStatus::ActionBroken);
    }

    TEST_CASE_E(stopable_sync_request)

    TEST_CASE_B(service_status) {
      EXPECT(serviceStatus == Availability::Unavailable);
    }
    TEST_CASE_E(service_status)
  }

 private:
  std::shared_ptr<Stub> stub_;
  std::shared_ptr<Proxy> proxy_;
};

static constexpr auto ServiceIDTest = "request_response_test.service";
void testLocalIPC() {
  using namespace localipc;
  maf::test::log_rec()
      << "--------------START LOCAL IPC TEST --------------------";
  Address addr{"maf.request_response_test.name", 0};

  auto stub = createStub(addr, ServiceIDTest);
  while (!stub) {
    std::this_thread::sleep_for(10ms);
    stub = createStub(addr, ServiceIDTest);
  };
  Tester<localipc::ParamTrait> tester{stub, createProxy(addr, ServiceIDTest)};
  tester.test();
}

void testITC() {
  using namespace itc;

  maf::test::log_rec() << "--------------START INTER THREAD COMMUNICATION TEST "
                          "--------------------";
  Tester<itc::ParamTrait> tester{createStub(ServiceIDTest),
                                 createProxy(ServiceIDTest)};
  tester.test();
}

int main() {
  //  maf::logging::init(maf::logging::LOG_LEVEL_FROM_INFO |
  //                         maf::logging::LOG_LEVEL_VERBOSE |
  //                         maf::logging::LOG_LEVEL_DEBUG,
  //                     [](const auto& msg) { std::cout << msg << std::endl;
  //                     });
  static maf::util::TimeMeasurement tm([](auto elapsed) {
    std::cout << "Total time = " << elapsed.count() / 1000 << "ms";
  });
  maf::test::init_test_cases();
  testLocalIPC();
  testITC();
  return 0;
}
